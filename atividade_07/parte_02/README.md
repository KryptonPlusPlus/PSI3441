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
```

## Respostas:

1. O `mutex` eliminou os problemas observados na Parte 1?

    Sim, O mutex eliminou as condições de corrida observadas na Parte 1, garantindo que apenas uma _thread_ por vez possa modificar a variável `count`. Entretanto, o mutex não controla limites lógicos da variável, como impedir que ela fique negativa ou ultrapasse uma capacidade máxima.

2. O `mutex` garante a consistência do valor de `count`?

    Sim, o `mutex` garante exclusão mútua durante o acesso à variável `count`, impedindo acessos concorrentes e evitando condições de corrida. Dessa forma, as operações de incremento e decremento ocorrem de forma consistente.

3. Qual recurso está sendo protegido pelo `mutex`?

    O recurso protegido é o uso e acesso da variável `count`.

4. Qual é a principal função de um `mutex` neste contexto?

    A principal função do `mutex` é garantir exclusão mútua no acesso ao recurso compartilhado, permitindo que apenas uma _thread_ por vez execute operações sobre a variável `count` e evitando condições de corrida.