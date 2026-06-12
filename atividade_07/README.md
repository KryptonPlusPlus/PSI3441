## Respostas:

1. Qual a diferença entre proteger um recurso compartilhado e controlar sua disponibilidade?

    Proteger um recurso compartilhado significa garantir que apenas uma thread por vez possa acessá-lo ou modificá-lo, evitando condições de corrida e inconsistências nos dados. Já controlar sua disponibilidade significa gerenciar quando um recurso pode ser utilizado, impedindo, por exemplo, que um consumidor consuma itens inexistentes ou que um produtor ultrapasse a capacidade máxima do sistema.

2. Em qual etapa da atividade o `mutex` foi suficiente?

    O `mutex` foi suficiente na Parte 2, onde o objetivo era apenas garantir a consistência do acesso à variável compartilhada `count`. Nesse caso, o `mutex` impediu acessos simultâneos e eliminou as condições de corrida.

3. Em qual etapa os semáforos se mostraram mais adequados?

    Os semáforos se mostraram mais adequados na Parte 3, pois além de garantir a sincronização entre produtor e consumidor, também controlaram a quantidade de itens disponíveis e os espaços livres. Dessa forma, impediram que o número de itens se tornasse negativo ou ultrapassasse a capacidade máxima definida.

4. Em uma aplicação real de produtor-consumidor, quando seria necessário utilizar mutexes, semáforos ou ambos? Justifique.

    Em aplicações reais, utiliza-se um `mutex` quando é necessário proteger estruturas de dados compartilhadas contra acessos simultâneos. Os semáforos são utilizados para controlar a disponibilidade de recursos, sincronizando produtores e consumidores. Em muitos casos é necessário utilizar ambos, os semáforos controlam quantos itens podem ser produzidos ou consumidos, enquanto o `mutex` protege a região crítica onde a estrutura compartilhada é acessada e modificada.