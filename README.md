# PSI3441

---

## Atividade 1 (LEDs)

### Problema:

Fazer um programa para piscar os **3 LEDs** como em um **semáforo** utilizando uma **máquina de estados**.

### Perguntas:

1. Por que os LEDs são _Active Low_ (acendem quando se coloca 0 na saída)? 
2. Quais funções você usou para acender e apagar os LEDs?
3. Explique o que é o _Device Tree_. 
4. Explique as abstrações feitas pelo Sistema Operacional.

## Atividade 2 (Controle Interativo de Cor e Intensidade)

### Problema:

Fazer um programa para que o LED da placa FRDM-KL25Z, exiba a **cor laranja** e seja possível variar a **intensidade** em tempo real pelo **terminal serial** (UART).

- **Ajuste de Cor:** Utilize canais de PWM para misturar as cores e obter o **tom laranja**. Ajuste a frequência do sinal para que a oscilação não seja perceptível ao olho humano.
- **Interatividade via Terminal:** O programa deve solicitar que o usuário digite um valor de **0 a 100** (porcentagem) no console.
- **Cálculo de Duty Cycle:** O código deve calcular o novo _duty cycle_ dos LEDs mantendo a proporção da cor laranja, mas alterando a intensidade total conforme o valor digitado.
- **Abstração de Hardware:** É obrigatório o uso do **_Device Tree_ (DTS)** e das APIs do Zephyr para acessar os controladores de PWM, evitando o uso de endereços ou pinos fixos (_hardcoded_) no meio do código principal.

## Atividade 3 (_blink bare metal_)

### Problema:

Desenvolva um programa, utilizando acesso direto aos registradores, para fazer o LED **verde** piscar com período de **2 segundos**.

Sequência de execução do programa:

```
1. Habilitar clock da porta B.

2. Configurar o registrador de controle do pino 19 (Pin Control Register).

3. Configurar o pino 19 como saída.

4. Acionar a saída do pino 19.

5. Executar a função de espera (utilizar a função fornecida: void delay_ms(int n), em que n representa o tempo em milissegundos).

6. Desacionar a saída do pino 19.

7. Executar novamente a função de espera.

8. Repetir continuamente os passos 4 a 7.
```

## Atividade 4 (_adc bare metal_)

### Problema:

Escreva um código em C configurando os registradores da FRDM-KL25Z para realizar as seguintes tarefas:
1. Efetuar uma aquisição analógica utilizando o conversor A/D (ADC).
2. Acender o LED **azul** quando a tensão medida estiver próxima de **3,3 V**;
2. Acender o LED **verde** quando a tensão medida estiver próxima de **0 V**.

## Atividade 5 (Biblioteca para Sensor Ultrassônico)

### Problema:
O módulo de ultrassom pode ser utilizado para medir distâncias por meio do envio e da recepção de ondas sonoras. Nesta atividade, você deverá desenvolver uma biblioteca para facilitar a utilização desse módulo em projetos futuros.

1. Geração do sinal de Trigger com PWM

    Crie o sinal de `trigger` utilizando um periférico de **PWM**. O sensor requer um pulso de, no mínimo, **10 μs** para iniciar uma medição.

    * Escolha uma frequência e um duty cycle adequados para gerar esse pulso.
    * Defina também a taxa de atualização das medições, ou seja, quantas vezes por segundo o sensor realizará uma nova medida de distância.
    * Observe que é possível alterar a fonte de clock do **PWM**. Considere utilizar uma fonte mais lenta (por exemplo, **8 MHz**) para facilitar a geração de períodos maiores entre as medições.

2. Medição da largura de pulso com Input Capture

    Utilizando o recurso de **Input Capture**, desenvolva um programa capaz de medir a largura de um pulso PWM gerado na etapa anterior.

    * Conecte a saída do PWM diretamente à entrada configurada para captura.
    * Valide o funcionamento comparando o valor medido com a largura de pulso configurada.

3. Medição de distância com o sensor ultrassônico

    Após validar o mecanismo de captura:

    * Conecte o sinal PWM ao pino **Trigger** do módulo de ultrassom.
    * Utilize o **Input Capture** para medir a duração do pulso presente no pino **Echo**.
    * A partir do tempo medido, calcule a distância até o objeto utilizando a velocidade do som.

4. Criação de uma biblioteca

    Depois de validar o funcionamento completo do sensor organize o código desenvolvido em uma biblioteca reutilizável.
    
## Atividade 6 (_Threads_)

### Problema:

Nesta atividade, você deverá integrar diferentes periféricos e utilizar múltiplas threads para executar tarefas concorrentes no sistema.

1. Integre ao projeto os exemplos de leitura do ADC, botão com interrupção e acelerômetro.
2. Implemente uma _thread _responsável pela leitura do ADC, executando periodicamente a cada `500 ms`. Os valores lidos devem ser enviados para a interface serial.
3. Implemente uma segunda thread responsável pela leitura do acelerômetro, executando periodicamente a cada `1000 ms`. Os valores dos eixos X, Y e Z devem ser exibidos na interface serial.
4. Configure o botão utilizando interrupção, conforme apresentado no tutorial.
5. A cada acionamento do botão, alterne entre os seguintes modos de operação:
    - Modo ADC: exibe somente os valores lidos pelo ADC.
    - Modo Completo: exibe os valores do ADC e do acelerômetro.
6. Inicialize as duas _threads_ utilizando a mesma prioridade de execução.
7. Em seguida, modifique as prioridades das threads, observe o comportamento do sistema e descreva os efeitos observados. Explique como o escalonador utiliza as prioridades para determinar a execução das tarefas.

## Atividade 7 (_Shared Resources_)

### Problema 

Esta atividade tem como objetivo explorar o compartilhamento de recursos entre múltiplas threads e a utilização de mecanismos de sincronização para evitar inconsistências e coordenar o acesso a recursos compartilhados.

Será implementado um sistema composto por:

- Uma _thread_ **produtora**, responsável por gerar itens.
- Uma _thread_ **consumidora**, responsável por processar ou remover itens.
- Um recurso compartilhado, representado por uma variável denominada `count`.

#### Parte 1 – Sem sincronização

- A _thread_ produtora incrementa o valor de contador a cada `1 segundo`.
- A _thread_ consumidora decrementa o valor de contador a cada `1,5 segundos`.
- O recurso compartilhado é representado pela variável contador.
    - Produção: `count++`
    - Consumo: `count--`
- Não utilize mutexes, semáforos, filas ou qualquer outro mecanismo de sincronização.

Execute o programa e responda:

1. O valor de `count` permanece consistente ao longo da execução?
2. O valor de `count` pode assumir valores inesperados?
3. O comportamento muda ao alterar os períodos de execução das _threads_?
4. O comportamento muda ao alterar as prioridades das _threads_?
5. Quais problemas podem ocorrer quando múltiplas _threads_ acessam simultaneamente um mesmo recurso compartilhado?

#### Parte 2 - Utilizando `Mutex`

Adicione um `mutex` para proteger o acesso à variável `count`.

Execute novamente o programa e responda:

1. O `mutex` eliminou os problemas observados na Parte 1?
2. O `mutex` garante a consistência do valor de `count`?
3. Qual recurso está sendo protegido pelo `mutex`?
4. Qual é a principal função de um `mutex` neste contexto?

#### Parte 3 - Utilizando Semáforos

Modifique o programa para utilizar semáforos.

Considere que:

1. O sistema possui um capacidade máxima de 10 itens.
2. A _thread_ consumidora só pode consumir quando houver itens disponíveis.
3. A _thread_ produtora só pode produzir quando houver espaço disponível.

Execute novamente o programa e responda:

1. O número de itens disponíveis pode se tornar negativo?
2. O número de itens pode ultrapassar a capacidade máxima definida?
3. Qual problema foi resolvido pelos semáforos que não era resolvido apenas pelo `mutex`?
4. Qual é a função de cada semáforo utilizado?

### Comparação

1. Qual a diferença entre proteger um recurso compartilhado e controlar sua disponibilidade?
2. Em qual etapa da atividade o `mutex` foi suficiente?
3. Em qual etapa os semáforos se mostraram mais adequados?
4. Em uma aplicação real de produtor-consumidor, quando seria necessário utilizar mutexes, semáforos ou ambos? Justifique.

---
