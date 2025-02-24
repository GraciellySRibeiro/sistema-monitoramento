# 📡 Sistema de Monitoramento de Estruturas

## 📌 Descrição
Este projeto implementa um **sistema de monitoramento de vibrações** em pontes e viadutos utilizando a **plataforma BitDogLab** e o **microcontrolador Raspberry Pi Pico**. O sistema detecta possíveis anomalias estruturais e emite alertas visuais e sonoros para indicar risco de falha na estrutura.

## 🎯 Objetivos
- Criar um **protótipo open-source** acessível e eficiente.
- Demonstrar a viabilidade de um sistema compacto para **monitoramento de infraestrutura**.
- Auxiliar na **prevenção de acidentes** em estruturas civis.

## 🛠 Componentes Utilizados
- **Microcontrolador:** Raspberry Pi Pico
- **Sensor de Vibração:** Simulado com um microfone da plataforma BitDogLab
- **Display OLED:** Para exibição das informações
- **Buzzer:** Para alertas sonoros
- **LED RGB:** Indica o status do monitoramento
- **Botões:** Para ativar/desativar o monitoramento

## ⚙️ Instalação e Configuração
### 📥 Requisitos
Antes de iniciar, certifique-se de ter os seguintes itens instalados:
- [CMake](https://cmake.org/download/)
- [SDK do Raspberry Pi Pico](https://github.com/raspberrypi/pico-sdk)
- [VS Code](https://code.visualstudio.com/) com extensão para desenvolvimento no Pico
- Python 3 instalado
- Git configurado

### 🔧 Compilação e Upload para o Raspberry Pi Pico
1. Clone este repositório:
   ```sh
   git clone https://github.com/GraciellySRibeiro/sistema-monitoramento.git
   cd sistema-monitoramento
   ```
2. Crie a pasta `build` e execute o CMake:
   ```sh
   mkdir build
   cd build
   cmake ..
   ```
3. Compile o código:
   ```sh
   make
   ```
4. Copie o arquivo `.uf2` gerado para o Raspberry Pi Pico.
   - Conecte o Pico ao computador segurando o botão **BOOTSEL**
   - Copie o arquivo `display_oled.uf2` para o dispositivo

## 🚀 Como Usar
1. **Pressione o botão A** para ativar o monitoramento.
2. O sistema **exibirá mensagens** no **display OLED** indicando o status.
3. Caso seja detectada uma vibração anormal:
   - O **LED vermelho** se acenderá ⚠️
   - O **buzzer** emitirá um alerta sonoro 🔊
   - O **display OLED** mostrará **"ALERTA: VIBRAÇÃO EXCESSIVA!"**
4. **Pressione o botão B** para **desativar** o sistema e limpar os alertas.



