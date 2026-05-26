# PSI3441

---

## Atividade 1 (leds)

### Problema:

Fazer um programa para piscar o 3 leds como em um semáforo utilizando uma máquina de estados.

### Perguntas:

1. Por que os leds são _Active Low_ (acendem quando se coloca 0 na saída)? 
2. Quais funções você usou para acender e apagar os leds?
3. Explique o que é o _Device Tree_. 
4. Explique as abstrações feitas pelo Sistema Operacional.

## Atividade 2 (Controle Interativo de Cor e Intensidade)

### Problema:

Fazer um programa para que o led da placa FRDM-KL25Z  exiba a cor laranja e seja possível variar a intensidade em tempo real pelo terminal serial (UART).

- **Ajuste de Cor:** Utilize canais de PWM para misturar as cores e obter o **tom laranja**. Ajuste a frequência do sinal para que a oscilação não seja perceptível ao olho humano.
- **Interatividade via Terminal:** O programa deve solicitar que o usuário digite um valor de **0 a 100** (porcentagem) no console.
- **Cálculo de Duty Cycle:** O código deve calcular o novo _duty cycle_ dos leds mantendo a proporção da cor laranja, mas alterando a intensidade total conforme o valor digitado.
- **Abstração de Hardware:** É obrigatório o uso do **_Device Tree_ (DTS)** e das APIs do Zephyr para acessar os controladores de PWM, evitando o uso de endereços ou pinos fixos (_hardcoded_) no meio do código principal.

## Atividade 3 (_blink bare metal_)

Escreva um programa para fazer o LED verde piscar com 2 segundo de período usando registradores.

Sequência do programa:

1. Habilitar clock da porta B.

2. Configurar Pino 19 (Pin Control Register).

3. Setar a direção do Pino.

4. Habilitar saída.

5. Função de espera (função espera dada, `delay_ms(n)`, n em milisegundos).

6. Desabilitar saída.

7. Função de espera.

8. Repetir passos 4-7.

---
