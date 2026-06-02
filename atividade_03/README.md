# Registradores usados

#### `SIM_SCGC5` (**Endereço:** `0x40048038`)

```c
SIM_SCGC5 |= 1 << 10;
```

*System Clock Gating Control Register 5*, tem a função de habilitar os *clocks* dos módulos dos PORTn (nesse caso do PORTB, *bit* 10).

| Campo | Descrição |
|---------|---------|
| bit 10 | *Port B Clock Gate Control* |
| PORTB | *This bit controls the clock gate to the Port B module.* |
| 0 | *Clock disabled* |
| **1** | *Clock enabled* |

#### `PORTB_PCR19` (**Endereço:** `0x4004A04C`)

```c
PORTB_PCR19 = 1 << 8;
```

*Pin Control Register n* (`n = 19` nesse caso), uma das funções é configurar o multiplexador de cada pino do PORTB definindo o recurso que será utilizado (nesse caso GPIO, portanto os *bits* 10:8 = `001₂`).

| Campo | Descrição |
|---------|---------|
| bits 10:8 | *Pin Mux Control* |
| `000₂` | *Pin disabled (analog)* |
| **`001₂`** | *Alternative 1 (GPIO)* |
| `010₂` | *Alternative 2 (chip-specific)* |
| `011₂` | *Alternative 3 (chip-specific)* |
| `100₂` | *Alternative 4 (chip-specific)* |
| `101₂` | *Alternative 5 (chip-specific)* |
| `110₂` | *Alternative 6 (chip-specific)* |
| `111₂` | *Alternative 7 (chip-specific)* |

#### `GPIOB_PDDR` (**Endereço:** `0x400FF054`)

```c
GPIOB_PDDR |= 1 << 19;
```

*Port Data Direction Register*, configura a direção de cada um dos pinos do PORTB, se vai ser *input* ou *output* (configura o *bit* 19 como 1, definindo o pino correspondente como saída digital).

| Campo | Descrição |
|---------|---------|
| bits 31:0 | *Port Data Direction* |
| 0 | *Pin is configured as general-purpose input* |
| **1** | *Pin is configured as general-purpose output* |

#### `GPIOB_PSOR` (**Endereço:** `0x400FF044`)

```c
GPIOB_PSOR = 1 << 19;
```

*Port Set Output Register*, define os pinos do PORTB como nível lógico alto (nesse caso apenas o *bit* 19), porém pelo fato dos LEDs serem *Active Low* eles são desligados com esse registrador.

| Campo | Descrição |
|---------|---------|
| bits 31:0 | *Port Set Output* |
| 0 | *Corresponding bit in PDORn does not change* |
| **1** | *Corresponding bit in PDORn is set to logic 1* |

#### `GPIOB_PCOR` (**Endereço:** `0x400FF048`)

```c
GPIOB_PCOR = 1 << 19;
```

*Port Clear Output Register*, define os pinos do PORTB como nível lógico baixo (nesse caso apenas o *bit* 19), porém pelo fato dos LEDs serem *Active Low* eles são ligados com esse registrador.

| Campo | Descrição |
|---------|---------|
| bits 31:0 | *Port Clear Output* |
| 0 | *Corresponding bit in PDORn does not change* |
| **1** | *Corresponding bit in PDORn is cleared to logic 0* |

Fonte: [1]

---

# Análise da função de atraso

O compilador otimizou o laço interno substituindo a condição original `j < 7000` por um contador regressivo iniciado em 7000 e decrementado até zero, reduzindo o número de instruções necessárias para o teste da condição de parada.

```c
   void delay_ms (int n) {
1.    int i;
2.    int j; 
3.    for (i = 0; i < n; i++)
4.        for (j = 0; j < 7000; j++)
5.            __asm volatile ("nop");
6. }
```

Foram obtidas as seguintes instruções geradas pelo compilador para a função `delay_ms`:

```asm
578:    movs    r3, #0          ; i = 0
57a:    cmp     r3, r0          ; compara i com n
57c:    blt.n   580             ; continua se i < n
57e:    bx      lr              ; retorno da função

580:    ldr     r2, [pc, #12]   ; carrega 7000 em j
582:    nop                     ; atraso definido na linha 5 da função
584:    subs    r2, #1          ; j--
586:    cmp     r2, #0          ; compara j com zero
588:    bne.n   582             ; repete enquanto j != 0

58a:    adds    r3, #1          ; i++
58c:    b.n     57a             ; repete laço externo

58e:    nop

590:    .word   0x00001b58      ; constante 7000

...

5a6:    movs    r5, #250
...
5b4:    lsls    r5, r5, #2      ; 250 << 2 = 1000
...
5c6:    movs    r0, r5
```

Nas linhas `5a6`, `5b4` e `5c6`, o compilador prepara o argumento da chamada à função `delay_ms`. Inicialmente, o valor decimal 250 é carregado no registrador `r5`; em seguida, a instrução `lsls` realiza um deslocamento lógico de dois bits para a esquerda, produzindo o valor 1000. Por fim, esse valor é copiado para o registrador `r0`, utilizado pela convenção de chamadas ARM para passagem do primeiro argumento de função.

Observa-se que as variáveis `i` e `j` foram mantidas inteiramente em registradores (`r3` e `r2`, respectivamente), sem necessidade de armazenamento na pilha. Essa otimização elimina acessos à memória durante as iterações, reduzindo o número de ciclos de execução da rotina de atraso.

Obtendo o número de ciclos para cada instrução, temos [2]:

| Instrução | Ciclos |
|------------|---------|
| `ldr` | 2 |
| `nop` | 1 |
| `subs` | 1 |
| `adds` | 1 |
| `cmp` | 1 |
| `bne` / `blt` | 1 ou 2* |
| `b` / `bx` | 2 |

\* 2 ciclos se o desvio for tomado, 1 ciclo caso contrário.

Portanto, para o laço interno são necessários 5 ciclos (linhas `582` a `588`) para as primeiras 6999 iterações, sendo que a última executa um ciclo a menos.

$$
   6999 \cdot \left(1_{nop} + 1_{subs} + 1_{cmp} + 2_{bne}\right) + 4 = 34999 \text{ ciclos}
$$

Sendo necessário adicionar mais dois ciclos para a instrução que carrega o valor 7000 para o `j` (`ldr r2, [pc, \#12]`), obtém-se:

$$
    34999 + 2 = 35001 \text{ ciclos}
$$

 Obtendo o tempo de uma iteração externa, acrescentando o controle do laço externo, considerando apenas as primeiras $n - 1$, em que o desvio `blt` é tomado, na última iteração o custo é ligeiramente menor, pois o desvio não é executado, temos:

$$
    35001 + 1_{adds} + 2_{b} + 1_{cmp} + 2_{blt} = 35007 \text{ ciclos}
$$

Sabendo que:

$$
    f_{cpu} = 48\text{ MHz}
$$

temos:

$$
    T_{ciclo} = \frac{1}{f_{cpu}} \approx 2.083 \cdot 10^{-8} = 20.83\text{ ns}
$$

Logo:

$$
    T_{ite} = 35007 \cdot 20.83 \cdot 10^{-9} \approx 0.729 \text{ ms}
$$

Considerando que o parâmetro n é igual a 1000:

```c
delay_ms(1000);
```

obtém-se:

$$
    T_{total} = 1000 \cdot  0.729 \cdot 10^{-3} = 729\text{ ms}
$$

Sendo obtido um erro de:

$$
    erro = \frac{1000 - 729}{1000} \cdot 100 \approx 27.1%
$$

Ou seja, para o *clock* de $48 \text{ MHz}$ configurado pelo *Zephyr*, o valor 7000 do laço interno não corresponde a $1\text{ ms}$.

A análise do código *assembly* mostrou que o compilador otimizou os laços utilizando registradores para as variáveis de controle e substituindo o contador crescente interno por um contador regressivo. Considerando a frequência de $48\text{ MHz}$ configurada pelo Zephyr e a quantidade de ciclos estimada para cada instrução, obteve-se um atraso aproximado de $729\text{ ms}$ para a chamada `delay_ms(1000)`. Assim, a constante 7000 utilizada no laço interno não produz um atraso de $1\text{ ms}$ por iteração nesse ambiente de execução, resultando em um erro aproximado de $27.1%$.

---

# Referências

[1] NXP Semiconductors. *KL25 Sub-Family Reference Manual*.

[2] ARM Ltd. *ARM Cortex-M0+ Technical Reference Manual*.

[3] Zephyr Project Documentation. Configuração da FRDM-KL25Z.