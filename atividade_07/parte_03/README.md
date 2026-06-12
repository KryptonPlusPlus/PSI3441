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
[00:00:12.002,000] <dbg> app: sub_thread_entry: [sub] count: 3
[00:00:12.002,000] <dbg> app: add_thread_entry: [add] count: 4
[00:00:13.002,000] <dbg> app: add_thread_entry: [add] count: 5
[00:00:13.502,000] <dbg> app: sub_thread_entry: [sub] count: 4
[00:00:14.003,000] <dbg> app: add_thread_entry: [add] count: 5
[00:00:15.002,000] <dbg> app: sub_thread_entry: [sub] count: 4
[00:00:15.003,000] <dbg> app: add_thread_entry: [add] count: 5
[00:00:16.003,000] <dbg> app: add_thread_entry: [add] count: 6
[00:00:16.502,000] <dbg> app: sub_thread_entry: [sub] count: 5
[00:00:17.003,000] <dbg> app: add_thread_entry: [add] count: 6
[00:00:18.002,000] <dbg> app: sub_thread_entry: [sub] count: 5
[00:00:18.003,000] <dbg> app: add_thread_entry: [add] count: 6
[00:00:19.004,000] <dbg> app: add_thread_entry: [add] count: 7
[00:00:19.503,000] <dbg> app: sub_thread_entry: [sub] count: 6
[00:00:20.004,000] <dbg> app: add_thread_entry: [add] count: 7
[00:00:21.003,000] <dbg> app: sub_thread_entry: [sub] count: 6
[00:00:21.004,000] <dbg> app: add_thread_entry: [add] count: 7
[00:00:22.004,000] <dbg> app: add_thread_entry: [add] count: 8
[00:00:22.503,000] <dbg> app: sub_thread_entry: [sub] count: 7
[00:00:23.004,000] <dbg> app: add_thread_entry: [add] count: 8
[00:00:24.003,000] <dbg> app: sub_thread_entry: [sub] count: 7
[00:00:24.005,000] <dbg> app: add_thread_entry: [add] count: 8
[00:00:25.005,000] <dbg> app: add_thread_entry: [add] count: 9
[00:00:25.503,000] <dbg> app: sub_thread_entry: [sub] count: 8
[00:00:26.005,000] <dbg> app: add_thread_entry: [add] count: 9
[00:00:27.004,000] <dbg> app: sub_thread_entry: [sub] count: 8
[00:00:27.005,000] <dbg> app: add_thread_entry: [add] count: 9
[00:00:28.005,000] <dbg> app: add_thread_entry: [add] count: 10
[00:00:28.504,000] <dbg> app: sub_thread_entry: [sub] count: 9
[00:00:29.006,000] <dbg> app: add_thread_entry: [add] count: 10
[00:00:30.004,000] <dbg> app: sub_thread_entry: [sub] count: 9
[00:00:30.006,000] <dbg> app: add_thread_entry: [add] count: 10
[00:00:31.504,000] <dbg> app: sub_thread_entry: [sub] count: 9
[00:00:31.504,000] <dbg> app: add_thread_entry: [add] count: 10
[00:00:33.004,000] <dbg> app: sub_thread_entry: [sub] count: 9
[00:00:33.005,000] <dbg> app: add_thread_entry: [add] count: 10
[00:00:34.505,000] <dbg> app: sub_thread_entry: [sub] count: 9
[00:00:34.505,000] <dbg> app: add_thread_entry: [add] count: 10
[00:00:36.005,000] <dbg> app: sub_thread_entry: [sub] count: 9
[00:00:36.005,000] <dbg> app: add_thread_entry: [add] count: 10
```

## Respostas:

1. O número de itens disponíveis pode se tornar negativo?

    Não, o semáforo que controla a quantidade de itens disponíveis impede que a _thread_ consumidora execute quando não houver itens para consumir. Dessa forma, o valor de `count` não pode se tornar negativo.

2. O número de itens pode ultrapassar a capacidade máxima definida?

    Não, o semáforo que controla os espaços disponíveis bloqueia a produção quando a capacidade máxima é atingida. Assim, o número de itens nunca ultrapassa o limite de 10 itens definido para o sistema.

3. Qual problema foi resolvido pelos semáforos que não era resolvido apenas pelo `mutex`?

    Enquanto o `mutex` apenas garante acesso exclusivo ao recurso compartilhado, os semáforos controlam a disponibilidade dos recursos. Eles impedem que a consumidora consuma quando não há itens e que a produtora produza quando a capacidade máxima foi atingida.

4. Qual é a função de cada semáforo utilizado?

    O semáforo de itens disponíveis controla quantos itens podem ser consumidos, bloqueando a consumidora quando não existem itens. O semáforo de espaços disponíveis controla a capacidade máxima do sistema, bloqueando a produtora quando não há mais espaço para novos itens.


