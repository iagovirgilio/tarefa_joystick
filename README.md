# Projeto Joystick com RP2040 e Display SSD1306

Este projeto demonstra o uso do conversor analógico-digital (ADC) do RP2040 para ler um joystick, controlar o brilho de LEDs RGB via PWM e exibir informações gráficas em um display OLED SSD1306 utilizando o protocolo I2C. O objetivo é consolidar os conhecimentos sobre ADC, PWM e comunicação I2C, além de explorar o uso de interrupções e debouncing para tratamento de botões.

---

## Funcionalidades

- **Leitura do Joystick**  
  Os valores analógicos dos eixos X e Y (usando os pinos ADC 26 e ADC 27, respectivamente) são lidos para:
  - Controlar o brilho dos LEDs vermelho e azul.  
  - Atualizar a posição de um quadrado de 8×8 pixels no display, representando a posição do joystick.
  
- **Controle dos LEDs via PWM**  
  Os LEDs vermelho e azul têm seu brilho ajustado conforme a posição do joystick. O LED verde alterna seu estado a cada pressionamento do botão do joystick.

- **Exibição Gráfica no Display SSD1306**  
  O display OLED mostra um quadrado que inicia centralizado e se move de acordo com o movimento do joystick. A borda do display também é alterada ao pressionar o botão do joystick.

- **Uso de Interrupções e Debouncing**  
  As funcionalidades dos botões (joystick e botão A) são gerenciadas por interrupções, com debouncing implementado via software para evitar leituras falsas.

- **Botão A**  
  Alterna a ativação do controle PWM dos LEDs.

---

## Componentes Utilizados

- **Placa de Desenvolvimento:** BitDogLab (RP2040)
- **Joysticks:**  
  - Eixo X: conectado ao ADC 26  
  - Eixo Y: conectado ao ADC 27  
  - Botão do Joystick: GPIO 22
- **LED RGB:**  
  - LED Vermelho: GPIO 13 (PWM)  
  - LED Azul: GPIO 12 (PWM)  
  - LED Verde: GPIO 11 (Saída Digital)
- **Botão A:**  
  - GPIO 5
- **Display OLED SSD1306:**  
  - SDA: GPIO 14  
  - SCL: GPIO 15  
  - Configuração I2C com endereço 0x3C

---

## Requisitos do Projeto

- **Hardware:**  
  - BitDogLab (RP2040)  
  - Joystick com botão  
  - LED RGB  
  - Botão A  
  - Display OLED SSD1306 (128×64)
  
- **Software:**  
  - SDK do Raspberry Pi Pico (RP2040 SDK)  
  - Compilador C/C++ para ARM  
  - Ferramentas de build (Make, CMake, etc.)

---

## Conexões de Hardware

- **Joystick:**  
  - Eixo X: ADC 26  
  - Eixo Y: ADC 27  
  - Botão: GPIO 22
- **LED RGB:**  
  - Vermelho: GPIO 13  
  - Azul: GPIO 12  
  - Verde: GPIO 11
- **Botão A:**  
  - GPIO 5
- **Display SSD1306:**  
  - SDA: GPIO 14  
  - SCL: GPIO 15

---

## Compilação e Upload

1. **Configuração do Ambiente:**  
   Certifique-se de ter o SDK do Raspberry Pi Pico configurado no seu ambiente.

2. **Preparação do Projeto:**  
   - Crie um diretório para o projeto e copie os arquivos fonte (por exemplo, `joystick.c`, `ssd1306.h`, `font.h`, etc.).
   - Configure o arquivo `CMakeLists.txt` de acordo com as diretrizes do SDK.

3. **Compilação:**  
   No terminal, navegue até o diretório do projeto e execute os seguintes comandos:


4. **Upload:**  
Conecte sua placa BitDogLab ao computador e copie o arquivo UF2 gerado para a placa.

---

## Como Usar o Projeto

- **Inicialização:**  
Ao iniciar, o quadrado é centralizado no display.

- **Movimentação do Joystick:**  
Ao mover o joystick, os LEDs vermelho e azul ajustam seu brilho conforme a posição, e o quadrado se desloca no display de acordo com os valores lidos.

- **Interação com Botões:**  
- Pressione o botão do joystick para alternar o estado do LED verde e mudar o estilo da borda do display.  
- Pressione o botão A para ativar ou desativar o controle PWM dos LEDs.

---

## Observações

- **Debouncing:**  
O tratamento de debouncing para os botões foi implementado com um intervalo de 200 ms para evitar múltiplas leituras.

- **Zona Morta:**  
Uma zona morta (DEADZONE) é utilizada para manter o quadrado centralizado quando o joystick estiver próximo à posição de repouso.

- **Ajustes e Calibração:**  
Caso seja necessário, ajustes na calibração do joystick e na tolerância (DEADZONE) podem ser implementados para compensar variações de hardware.

---
