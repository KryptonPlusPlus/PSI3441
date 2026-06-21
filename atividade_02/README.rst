Atividade 02
============
.. collapse::
    Correções de bugs da versão do Zephyr utilizada

    No arquivo ``fsl_tpm.h``, localizado em  ``~/.platformio/packages/framework-zephyr/_pio/modules/hal/nxp/mcux/mcux-sdk-ng/drivers/tpm/fsl_tpm.h``, adicionar nas primeiras linhas, após os ``#include``, o seguinte bloco de código:

    .. code-block:: c

        #ifndef FSL_FEATURE_TPM_HAS_32BIT_COUNTERn
        #define FSL_FEATURE_TPM_HAS_32BIT_COUNTERn(x) (0)
        #endif

    No arquivo ``pwm_mcux_tpm.c``, localizado em  ``~/.platformio/packages/framework-zephyr/drivers/pwm/pwm_mcux_tpm.c``, corrigir o erro, na linha **64**, da variável ``base`` não definida, dentro da função ``mcux_tpm_set_cycles``, com o seguinte código:

    .. code-block:: c

        if (period_cycles == 0 || period_cycles == TPM_MAX_COUNTER_VALUE(base)) {

    Para:

    .. code-block:: c

        if (period_cycles == 0 || period_cycles == TPM_MAX_COUNTER_VALUE(config->base)) {


Análise de implementação da atividade
-------------------------------------

Abstração de Hardware
~~~~~~~~~~~~~~~~~~~~~

Os controladores PWM para os canais de cor vermelha e verde são referenciados via ``DT_ALIAS`` e associados a estruturas de controle estáticas por meio de ``PWM_DT_SPEC_GET``. A interface de comunicação UART também é instanciada dinamicamente utilizando a macro ``DEVICE_DT_GET`` apontando para o nó ``uart0``. 

Contudo, é possível observar uma exceção explícita na implementação para atender a uma particularidade da placa FRDM-KL25Z: o roteamento de *clock* para o módulo PWM não ocorreu via *Device Tree*, exigindo uma configuração manual por meio do registrador ``SIM_SOPT2`` diretamente no endereço de memória :math:`0x40048004` para selecionar a fonte de *clock* (``MCGFLLCLK``), contornando uma limitação temporária da árvore de dispositivos dessa placa específica.

No arquivo ``frdm_kl25z.overlay`` é realizada a abstração de hardware da configuração dos registradores e interrupções do PWM.

O nó padrão ``leds`` da placa foi explicitamente desabilitado para evitar conflitos entre o controle convencional dos LEDs via GPIO e o novo controle baseado em PWM. Dessa forma, os mesmos pinos podem ser reutilizados exclusivamente pelos periféricos TPM responsáveis pela geração do sinal PWM.

A configuração dos grupos ``tpm0_default`` e ``tpm2_default`` realiza o multiplexamento dos pinos físicos da placa para as funções alternativas dos módulos TPM. Dessa forma, os pinos deixam de operar como GPIO convencionais e passam a ser controlados diretamente pelo periférico PWM.

Os canais PWM foram configurados com ``PWM_POLARITY_INVERTED``, refletindo a característica *Active Low* dos LEDs RGB presentes na frdm-kl25z. Assim, pulsos de maior largura resultam em maior luminosidade percebida, apesar da lógica invertida aplicada ao pino físico.

Embora a implementação receba diretamente o período do PWM por meio do *Device Tree*, é interessante verificar a frequência correspondente e a compatibilidade do valor calculado de period com o contador de 16 bits do TPM. Para calcular a frequência necessária é preciso realizar a seguinte conta:

.. math::
    f_{pwm} = \frac{f_{core}}{\text{period} \cdot \text{prescale}} \Rightarrow 500 = \frac{24 \cdot 10^6}{\text{period} \cdot 128} \Rightarrow \text{period} = 375 < 65535

Como o TPM utiliza um contador de 16 bits, o valor calculado (:math:`375`) encontra-se confortavelmente dentro do limite máximo suportado (:math:`65535`). 

A frequência de **500 Hz** representa um compromisso adequado entre qualidade visual e resolução do PWM. Nessa frequência, a cintilação do LED torna-se imperceptível ao olho humano, enquanto o temporizador ainda dispõe de resolução suficiente para realizar ajustes finos no *duty cycle*.

Interatividade via Terminal
~~~~~~~~~~~~~~~~~~~~~~~~~~~

A **interatividade via terminal** ocorre através de uma rotina de recepção baseada em interrupções (IRQ), evitando que a CPU permaneça bloqueada aguardando dados de entrada. A função ``uart_callback`` é acionada toda vez que ocorre tráfego na interface serial. Utilizando ``uart_fifo_read``, os caracteres informados pelo usuário são enfileirados de byte em byte em um vetor local (``receive_buffer``) limitado a quatro posições, suficiente para armazenar valores percentuais de até três dígitos ("100"), além do terminador nulo.

Ao identificar o caractere de retorno de carro (``\r``) ou quebra de linha (``\n``), a rotina injeta o terminador de *string* ``\0``, reseta o índice do *buffer* e levanta a *flag* booleana volátil ``data_received``. Esse mecanismo sinaliza assincronamente à rotina principal (função ``main``) que uma *string* completa está pronta, sendo então convertida para um valor inteiro através de ``atoi`` e validada para assegurar que se encontra estritamente na faixa percentual de 0 a 100.


Cálculo de *duty cycle* e ajuste de Cor
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Por fim, a lógica une o cálculo de *duty cycle* ao ajuste de cor de modo interdependente no processamento final do laço principal. Para produzir uma tonalidade alaranjada em um LED RGB, é necessário que a contribuição luminosa do canal vermelho seja superior à do canal verde. O programa calcula primeiramente o ciclo de trabalho base em proporção direta com o período definido na especificação PWM obtida do *Device Tree* (mantendo a frequência constante e livre de oscilações indesejadas) por meio da seguinte equação:

.. math::
    \text{DutyCycle}_{base} = \frac{\text{Period} \cdot \text{Percentage}}{100}


Para forçar a mistura de cores para o laranja mantendo a proporcionalidade do brilho em qualquer percentual inserido no terminal, o código injeta a intensidade integral no canal vermelho por meio de ``pwm_set_pulse_dt(&led_red, duty_red)`` e, simultaneamente, impõe um divisor matemático no acionamento do canal verde utilizando ``pwm_set_pulse_dt(&led_green, duty_green / 2)``. 

Essa arquitetura garante que um eventual *input* de 50%, por exemplo, imponha meia largura de pulso no componente vermelho e um quarto da largura de pulso no componente verde. Isso reduz linearmente a potência luminosa global do componente, mas preserva intacta a composição da coloração alaranjada solicitada.