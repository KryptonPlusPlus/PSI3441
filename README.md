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
---

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
---
