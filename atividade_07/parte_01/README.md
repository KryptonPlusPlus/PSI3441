## Saída inicial da _log_:
```apache
*** Booting Zephyr OS build zephyr-v40200 ***
[00:00:00.000,000] <dbg> app: add_thread_entry: [add] count: 1
[00:00:00.000,000] <dbg> app: sub_thread_entry: [sub] count: 0
[00:00:01.000,000] <dbg> app: add_thread_entry: [add] count: 1
[00:00:01.500,000] <dbg> app: sub_thread_entry: [sub] count: 0
[00:00:02.000,000] <dbg> app: add_thread_entry: [add] count: 1
[00:00:03.000,000] <dbg> app: sub_thread_entry: [sub] count: 0
[00:00:03.000,000] <dbg> app: add_thread_entry: [add] count: 1
[00:00:04.001,000] <dbg> app: add_thread_entry: [add] count: 2
[00:00:04.500,000] <dbg> app: sub_thread_entry: [sub] count: 1
[00:00:05.001,000] <dbg> app: add_thread_entry: [add] count: 2
[00:00:06.001,000] <dbg> app: sub_thread_entry: [sub] count: 1
[00:00:06.001,000] <dbg> app: add_thread_entry: [add] count: 2
[00:00:07.001,000] <dbg> app: add_thread_entry: [add] count: 3
[00:00:07.501,000] <dbg> app: sub_thread_entry: [sub] count: 2
[00:00:08.001,000] <dbg> app: add_thread_entry: [add] count: 3
[00:00:09.001,000] <dbg> app: sub_thread_entry: [sub] count: 2
[00:00:09.002,000] <dbg> app: add_thread_entry: [add] count: 3
[00:00:10.002,000] <dbg> app: add_thread_entry: [add] count: 4
[00:00:10.501,000] <dbg> app: sub_thread_entry: [sub] count: 3
[00:00:11.002,000] <dbg> app: add_thread_entry: [add] count: 4
[00:00:12.001,000] <dbg> app: sub_thread_entry: [sub] count: 3
[00:00:12.002,000] <dbg> app: add_thread_entry: [add] count: 4
[00:00:13.002,000] <dbg> app: add_thread_entry: [add] count: 5
```

## Respostas:

1. O valor de `count` permanece consistente ao longo da execução?

    Não, em algumas chamadas das _threads_, quando acontece de ambas acordarem ao mesmo tempo, fica inconsistente qual tarefa ocorre primeiro, por exemplo, em `0.0 s` ambas as _threads_ são chamadas e ocorre primeiro `add` e em seguida `sub` e em `3.0 s` novamente ambas as _threads_ são chamadas e ocorre  primeiro `sub` e em seguida `add`, tornando imprevisivel o comportamento.

    Outro problema ocorre pelo fato de que as operações de incremento e decremento não são operações atômicas, inernamente elas resolvem as operações na seguinte ordem:

    1. Ler o valor.
    2. Modifica o valor.
    3. Escreve o valor.

    Se uma _thread_ interromper a outra durante esse processo, pode ocorrer uma condição de corrida  (_race condition_), resultando em perda de atualizações.

    Na execução observada não foi identificado nenhum comportamento incorreto ou evidência de condição de corrida. Entretanto, como a variável `count` é acessada simultaneamente por múltiplas _threads_ sem mecanismos de sincronização, o programa permanece sujeito a condições de corrida, que podem se manifestar em outras execuções ou configurações do sistema.

2. O valor de `count` pode assumir valores inesperados?

    Sim, como tem acesso concorrente sem sincronização, podem ocorrer condições de corrida que fazem incrementos ou decrementos serem perdidos. Assim, o valor de `count` pode ser diferente do esperado pela lógica do programa.

    Outro problema é dado pelo fato de que a variável `count` é representada pelo tipo `int8_t` ela pode acabar excedendo um valor limite. Outro valor inesperado seria se a variavel não pudesse assumir valores negativos e, por exemplo, na primeira chamada das tarefas ocorre-se a subtração antes da adição.

3. O comportamento muda ao alterar os períodos de execução das _threads_?

    Sim, alterar os períodos modifica a frequência com que cada _thread_ acessa a variável compartilhada. Se a consumidora executar mais frequentemente que a produtora, o valor tenderá a diminuir. Além disso, mudanças nos períodos alteram a probabilidade de ocorrerem acessos simultâneos e, consequentemente, condições de corrida.

4. O comportamento muda ao alterar as prioridades das _threads_?

    Sim, _threads_ com prioridade maior tendem a ser escalonadas antes das demais. Isso pode alterar a ordem de acesso à variável compartilhada e, consequentemente, os valores observados durante a execução. Entretanto, o problema de condição de corrida continua existindo, pois a prioridade não fornece sincronização.

5. Quais problemas podem ocorrer quando múltiplas _threads_ acessam simultaneamente um mesmo recurso compartilhado?

    Podem ocorrer condições de corrida, perda de atualizações, leitura de valores inconsistentes e comportamento não determinístico. Como o resultado depende da ordem de execução das _threads_, diferentes execuções do programa podem produzir resultados distintos.