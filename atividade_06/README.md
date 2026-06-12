### Análise do programa

O funcionamento se divide em três frentes principais que operam de forma concorrente. A _thread_ do acelerômetro tenta ler os dados dos eixos X, Y e Z e imprimir esses valores no _log_ a cada `1000 ms`, mas ela apenas toma essa ação se o botão pressionado (controlado por um semáforo e uma variável `atomic_t`). Paralelamente, a thread do ADC configura o canal analógico e realiza medições de tensão a cada `500 ms` milissegundos, convertendo o valor bruto para milivolts e imprimindo-o no _log_ de forma independente. O botão atua como o controlador do sistema, ele está configurado para disparar uma interrupção de _hardware_ na borda de descida, ou seja, ao ser pressionado. Quando isso ocorre, o sistema inverte o estado da variável de controle, funcionando como um interruptor liga e desliga para a exibição dos dados do acelerômetro. A função principal do programa apenas inicializa o botão e coloca o sistema em um estado de repouso permanente, deixando o controle total da execução para as _threads_.

Como elas estão no mesmo nível de prioridade, o escalonador as trata de forma igualitária. Uma _thread_ é executada até liberar o processador voluntariamente ao chamar a função de suspensão. Devido aos diferentes tempos de espera, elas raramente competem pela CPU ao mesmo tempo. Se acordarem no exato mesmo instante, a ordem dependerá de qual entrou na fila de prontos primeiro, mas uma não interromperá a outra de forma forçada.
 
Se o código for corrigido para que o acelerômetro tenha prioridade um e o ADC utilize prioridade dois, por exemplo, o cenário muda. Nesse caso, o acelerômetro passa a ter uma prioridade mais alta. Se a _thread_ do ADC estiver no meio de sua execução e a thread do acelerômetro acordar de seu período de espera, o sistema operacional pausará imediatamente o ADC, transferirá a CPU para o processamento do acelerômetro e só a devolverá quando este voltar a dormir. Na prática do código, como ambas as _threads_ realizam operações rápidas e passam a maior parte do tempo ociosas, essa mudança de prioridade não gerará uma diferença perceptível no _log_ de saída. Contudo, caso a função de espera do acelerômetro fosse removida, criando um processamento ininterrupto, a _thread_ do ADC com menor prioridade nunca mais seria executada pelo sistema.

### Análise do funcionamento

Para analisar o comportamento do escalonador quando tarefas com diferentes prioridades competem pelo processador, foi adicionado um atraso artificial na função `accel_thread_entry()` dentro do laço principal (`while(1)`) da função, conforme mostrado a seguir:

```C
for(int i = 0; i < 1000000; i++)
    __asm volatile ("nop");
```

Poderia ser utilizada a função `k_busy_wait()`.

Dessa forma, o tempo de execução da tarefa do acelerômetro é aumentado, facilitando a observação de trocas de contexto durante os testes. Tornando mais fácil observar as trocas de contexto e as decisões tomadas pelo escalonador durante a execução do sistema.
    
1. Acelerômetro com prioridade inferior ao ADC

    Nesse cenário, a tarefa do ADC possui prioridade superior à tarefa do acelerômetro. Assim, caso ambas estejam prontas a executar simultaneamente, o escalonador concederá o processador ao ADC.

    Quando a tarefa ADC desperta enquanto a tarefa ACC está em execução, ocorre uma troca de contexto. A tarefa ACC é interrompida temporariamente e o processador passa a executar a tarefa ADC. Após concluir sua execução e entrar novamente em estado de espera, o controle retorna para a tarefa ACC, que continua a execução a partir do ponto onde foi interrompida.

    Esse comportamento pode ser observado no trecho de _log_ a seguir:

    ```log
    [00:00:00.020,000] <dbg> app: adc_thread_entry:   [ADC] acordou
    [00:00:00.020,000] <inf> app: adc: 0 mV
    [00:00:00.020,000] <dbg> app: adc_thread_entry:   [ADC] dormiu
    [00:00:00.503,000] <dbg> app: accel_thread_entry: [ACC] acordou
    [00:00:00.503,000] <dbg> app: accel_thread_entry: [ACC] inicio sensor_sample_fetch
    [00:00:00.504,000] <dbg> app: accel_thread_entry: [ACC] fim sensor_sample_fetch
    [00:00:00.520,000] <dbg> app: adc_thread_entry:   [ADC] acordou
    [00:00:00.520,000] <inf> app: adc: 0 mV
    [00:00:00.521,000] <dbg> app: adc_thread_entry:   [ADC] dormiu
    [00:00:00.609,000] <inf> app: x: 9.595960, y: 0.354341, z: 0.746991
    [00:00:00.609,000] <dbg> app: accel_thread_entry: [ACC] dormiu
    ```

    Observa-se que a tarefa ADC entrou em estado de espera em `0.020 s`, devendo despertar novamente aproximadamente `500 ms` depois, ou seja, em `0.520 s`. Como o ADC possui prioridade superior, ele obtém imediatamente o processador ao despertar, mesmo com a tarefa ACC em execução.

2. Acelerômetro com prioridade superior ao ADC

    Nesse cenário, a tarefa do acelerômetro possui prioridade superior à tarefa do ADC. Portanto, sempre que ambas estiverem prontas para execução, a tarefa ACC terá preferência no uso do processador.

    O trecho de log a seguir demonstra esse comportamento:

    ```log
    [00:00:00.014,000] <dbg> app: adc_thread_entry:   [ADC] acordou
    [00:00:00.014,000] <inf> app: adc: 0 mV
    [00:00:00.015,000] <dbg> app: adc_thread_entry:   [ADC] dormiu
    [00:00:00.495,000] <dbg> app: accel_thread_entry: [ACC] acordou
    [00:00:00.495,000] <dbg> app: accel_thread_entry: [ACC] inicio sensor_sample_fetch
    [00:00:00.496,000] <dbg> app: accel_thread_entry: [ACC] fim sensor_sample_fetch
    [00:00:00.600,000] <inf> app: x: 9.605537, y: 0.258574, z: -1.015142
    [00:00:00.600,000] <dbg> app: accel_thread_entry: [ACC] dormiu
    [00:00:00.600,000] <dbg> app: adc_thread_entry:   [ADC] acordou
    ```

    Nesse caso, a tarefa ADC entrou em espera em `0.015 s`, portanto deveria despertar novamente em aproximadamente `0.515 s`. Entretanto, nesse instante a tarefa ACC ainda estava executando e possuía prioridade superior. Dessa forma, a tarefa ADC permaneceu pronta para execução, mas sem acesso ao processador.

    Somente quando a tarefa ACC entrou em estado de espera, em `0.600 s`, o escalonador pôde selecionar a tarefa ADC para execução.

    - Influência da função `sensor_sample_fetch()`

    Durante os testes foi observado um comportamento adicional relacionado à função `sensor_sample_fetch()`:

    ```log
    [00:00:00.146,000] <dbg> app: adc_thread_entry:   [ADC] dormiu
    [00:00:00.645,000] <dbg> app: accel_thread_entry: [ACC] acordou
    [00:00:00.645,000] <dbg> app: accel_thread_entry: [ACC] inicio sensor_sample_fetch
    [00:00:00.646,000] <dbg> app: adc_thread_entry:   [ADC] acordou
    [00:00:00.646,000] <inf> app: adc: 0 mV
    [00:00:00.646,000] <dbg> app: adc_thread_entry:   [ADC] dormiu
    [00:00:00.646,000] <dbg> app: accel_thread_entry: [ACC] fim sensor_sample_fetch
    [00:00:00.751,000] <inf> app: x: 9.576806, y: 0.162805, z: 0.766145
    [00:00:00.751,000] <dbg> app: accel_thread_entry: [ACC] dormiu
    ```

    Esse comportamento ocorre porque a função `sensor_sample_fetch()` pode bloquear temporariamente a tarefa que a executa enquanto aguarda a conclusão de operações do driver ou da comunicação com o sensor. Durante esse período, a tarefa ACC deixa de ocupar o processador e o escalonador pode selecionar outra tarefa pronta para execução, mesmo que ela possua prioridade inferior.

    Portanto, não ocorre uma preempção da tarefa ACC pela tarefa ADC. Na realidade, a tarefa ACC entra em estado de bloqueio, permitindo que a tarefa ADC utilize o processador temporariamente.

3. Acelerômetro e ADC com a mesma prioridade

    Quando ambas as tarefas possuem a mesma prioridade, nenhuma delas possui preferência sobre a outra. O comportamento observado passa a depender da configuração do escalonador, especialmente do mecanismo de _time slicing_.

    Caso o _time slicing_ esteja desabilitado, a tarefa que estiver executando tende a manter o processador até entrar em estado de espera ou concluir sua execução. Esse comportamento pode ser observado no _log_ abaixo:
    
    ```log
    [00:00:00.119,000] <dbg> app: adc_thread_entry:   [ADC] dormiu
    [00:00:00.539,000] <dbg> app: accel_thread_entry: [ACC] acordou
    [00:00:00.540,000] <dbg> app: accel_thread_entry: [ACC] inicio sensor_sample_fetch
    [00:00:00.541,000] <dbg> app: accel_thread_entry: [ACC] fim sensor_sample_fetch
    [00:00:00.645,000] <inf> app: x: 9.586383, y: 0.153228, z: 0.727838
    [00:00:00.645,000] <dbg> app: accel_thread_entry: [ACC] dormiu
    [00:00:00.645,000] <dbg> app: adc_thread_entry:   [ADC] acordou
    ```

    Observa-se que a tarefa ADC deveria ter despertado aproximadamente em `0.619 s`, porém sua execução só ocorreu após a tarefa ACC entrar em estado de espera.

    Da mesma forma que no caso anterior, também foi observado o efeito do bloqueio interno da função `sensor_sample_fetch()`:

    ```log
    [00:00:00.146,000] <dbg> app: adc_thread_entry:   [ADC] dormiu
    [00:00:00.645,000] <dbg> app: accel_thread_entry: [ACC] acordou
    [00:00:00.645,000] <dbg> app: accel_thread_entry: [ACC] inicio sensor_sample_fetch
    [00:00:00.646,000] <dbg> app: adc_thread_entry:   [ADC] acordou
    [00:00:00.646,000] <inf> app: adc: 0 mV
    [00:00:00.646,000] <dbg> app: adc_thread_entry:   [ADC] dormiu
    [00:00:00.646,000] <dbg> app: accel_thread_entry: [ACC] fim sensor_sample_fetch
    [00:00:00.751,000] <inf> app: x: 9.576806, y: 0.162805, z: 0.766145
    [00:00:00.751,000] <dbg> app: accel_thread_entry: [ACC] dormiu
    ```

    Novamente, a execução da tarefa ADC não ocorre por possuir a mesma prioridade da ACC, mas porque a tarefa ACC encontra-se temporariamente bloqueada durante a execução de `sensor_sample_fetch()`, permitindo que o escalonador selecione outra tarefa apta para execução.