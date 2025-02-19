# Resumo do Projeto

Este projeto utiliza um joystick analógico para controlar um display OLED SSD1306 e ajustar a intensidade de LEDs RGB. O movimento do joystick altera a posição de um cursor no display, enquanto os LEDs mudam de intensidade com base nos valores lidos dos eixos X e Y. Além disso, há botões que ativam/desativam os LEDs e alternam entre uma borda sólida e pontilhada no display. O projeto utiliza a Raspberry Pi Pico e faz uso de ADCs, PWM, I2C e interrupções para garantir um funcionamento responsivo.

# Instruções de Uso

Para compilar e executar o software, siga os passos abaixo:

## 1. Clonar o Repositório

Clone o repositório para o seu computador usando o comando:

```sh
git clone https://github.com/Kauanteles/tarefa-adc.git
cd tarefa-adc
```

Se estiver usando o **VSCode**, a extensão do **CMake** irá criar a pasta `build` automaticamente. Caso contrário, crie-a manualmente e execute:

```sh
mkdir build && cd build
cmake ..
```

Isso gerará os arquivos necessários para compilação.

## 2. Compilar o Firmware

Compile o firmware utilizando a extensão ***Raspberry Pi Pico*** do ***VSCode*** ou execute:

```sh
make -j4
```

Após a compilação, será gerado um arquivo `.uf2` que poderá ser gravado na placa.

# Execução no Raspberry Pi Pico

Para rodar o firmware na placa, siga os passos:

1. Coloque o Raspberry Pi Pico em modo **BOOTSEL** pressionando o botão branco na placa e conectando-a ao computador via USB.
2. Copie o arquivo `.uf2` da pasta `build` para o armazenamento do Pico (ele aparecerá como um dispositivo USB chamado `RPI-RP2`).
3. Após a cópia, o Pico reiniciará automaticamente e o firmware entrará em execução.

# Controles e Funcionalidades

- **Movimento do Joystick:** Controla a posição do cursor no display OLED.
- **Botão do Joystick:** Alterna entre borda sólida e borda pontilhada no display.
- **Botão A:** Liga ou desliga a funcionalidade dos LEDs RGB.
- **Movimento Vertical do Joystick:** Controla a intensidade do LED azul.
- **Movimento Horizontal do Joystick:** Controla a intensidade do LED vermelho.

# Vídeo de Demonstração

Assista ao funcionamento do projeto no seguinte link: ***[Vídeo do projeto](https://youtu.be/MOHmM3yGuBs)***.

