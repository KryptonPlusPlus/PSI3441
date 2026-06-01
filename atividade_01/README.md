### 1. Por que os LEDSs são _Active Low_ (acendem quando se coloca 0 na saída)?

Na eletrônica dos microcontroladores, os pinos de saída geralmente possuem uma capacidade consideravelmente maior de drenar corrente para o terra (_current sink_) do que de fornecer corrente a partir da fonte de alimentação (_current source_). 
    
Portanto ligar os LEDs na configuração _Active Low_ reduz o estresse elétrico no _chip_ e previne danos físicos às portas do microcontrolador. Além disso, essa configuração garante estabilidade, evitando que o LED pisque ou acenda acidentalmente devido a ruídos no barramento durante o processo de inicialização do sistema, momento em que os pinos frequentemente assumem um estado de alta impedância.

---
### 2. Quais funções você usou para acender e apagar os LEDs?

Para configuração e verificação de como os LEDs devem se comportar foram utilizadas as seguintes funções:
    
    
- `gpio_pin_configure_dt()`: Configura os pinos dos LEDs como saída e define o estado padrão.
        
    
- `gpio_is_ready_dt()`: Utilizada como etapa de validação de segurança para confirmar se o periférico mapeado está pronto para receber comandos do sistema.
    
Para acionamento dos LEDs foram utilizadas as seguintes funções:
    
- `gpio_pin_toggle_dt()`: Inverte o nível lógico no pino.
    
- `gpio_pin_set_dt()`: Utilizada para acender e apagar os LEDs de fato, alterando o estado lógico do pino (o valor passado como parâmetro representa o estado lógico desejado para o pino, sendo interpretado conforme a configuração do _hardware_, por exemplo, _Active Low_ ou _Active High_). 
    
---
### 3. Explique o que é o _Device Tree_

O _DeviceTree_ é uma estrutura de dados que descreve o _hardware_ da plataforma de forma declarativa. Em vez de utilizar endereços ou identificadores fixos no código, ele permite referenciar periféricos por meio de aliases e propriedades, com isso um pino com uma função especifica, por exemplo o LED, pode funcionar em diferentes plataformas com arranjos dos pinos diferentes. E é capaz de descrever propriedades dos pinos, por exemplo a polaridade do LED já que nessa placa ele é ativo em nível lógico baixo. Além de ser capaz de fazer verificações de segurança e portabilidade em tempo de compilação.

---
### 4. Explique as abstrações feitas pelo Sistema Operacional

Sistemas operacionais de tempo real como o _Zephyr_ constroem camadas de abstração de hardware para isolar o desenvolvedor da manipulação direta de registradores e mapeamento da memória por meio do _linker_.