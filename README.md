# FreeRTOS Desafio - Raspberry Pi Pico

Um sistema completo de controle de LED RGB e display OLED implementado para o
microcontrolador Raspberry Pi Pico utilizando FreeRTOS, com comunicação entre tarefas
através de Event Groups, Queues e Mutexes.

## 📋 Descrição do Sistema

O sistema implementa um controle interativo de LED RGB que utiliza um botão físico
para alternar cores, exibindo o estado do botão e a cor atual em um display OLED.
O projeto demonstra o uso de múltiplas tarefas FreeRTOS que se comunicam de forma
sincronizada através de mecanismos de IPC (Inter-Process Communication).

## ✨ Funcionalidades

-   **Leitura de Botão**: Tarefa dedicada que detecta mudanças de estado (ON/OFF)
-   **Controle de LED RGB**: Alternância automática de cores na sequência vermelho-verde-azul
-   **Display OLED**: Exibição em tempo real do estado do botão e cor atual do LED
-   **Sincronização de Tarefas**: Uso de Event Groups para comunicação assíncrona
-   **Fila de Mensagens**: Queue para transmissão de strings entre tarefas
-   **Proteção de Recursos**: Mutex para garantir acesso exclusivo ao display OLED
-   **Feedback Visual**: Atualização instantânea da interface conforme interações do usuário

## 🔧 Componentes de _Hardware_

### _On-board_
-   **Raspberry Pi Pico**
-   **Display OLED SSD1306** (128x64 pixels, I2C)
-   **Botão Push** (para controle do LED)
-   **LED RGB** (Vermelho, Verde e Azul)

### 🔧 Detalhes Técnicos

-   **Linguagem**: C (SDK Raspberry Pi Pico)
-   **RTOS**: FreeRTOS com suporte para RP2040
-   **Display**: SSD1306 OLED via I2C (400kHz, endereço 0x3C)
-   **LED RGB**: 3 GPIOs individuais (Vermelho: GPIO 13, Verde: GPIO 11, Azul: GPIO 12)
-   **Botão**: GPIO 5 com pull-up interno
-   **Comunicação entre Tarefas**: Event Groups, Queues e Mutexes
-   **Timing**: Delay de 50ms entre leituras do botão usando `pdMS_TO_TICKS()`

### 📊 Especificações do FreeRTOS (Componente Central)

O sistema utiliza o FreeRTOS com as seguintes características:
- **Event Group**: 3 bits (BUTTON_ON_BIT_1, BUTTON_ON_BIT_2, BUTTON_OFF_BIT)
- **Queue**: 5 elementos de 10 bytes cada (strings até 9 caracteres + NULL)
- **Mutex**: Proteção de acesso exclusivo ao display OLED
- **Tarefas**: 4 tarefas independentes com diferentes prioridades
- **Timeout de Queue**: 100ms para envio de mensagens (`pdMS_TO_TICKS(100)`)
- **Timeout de Event Group**: Indefinido (`portMAX_DELAY`) para tarefa de leitura de cor

### 🏗️ Arquitetura do Sistema

### Diagrama de Blocos

**Componentes Principais:**

- **Microcontrolador**: Raspberry Pi Pico (RP2040)
- **RTOS**: FreeRTOS Kernel
- **Display**: OLED SSD1306 via I2C
- **LED RGB**: 3 pinos GPIO individuais
- **Botão**: GPIO com pull-up interno
- **Mecanismos IPC**: Event Group, Queue, Mutex

**Conexões entre RP2040 e Periféricos**
- **GPIO 2-3**: Display OLED (Módulo I2C1 - SDA/SCL)
- **GPIO 5**: Botão (entrada com pull-up interno)
- **GPIO 11**: LED Verde
- **GPIO 12**: LED Azul
- **GPIO 13**: LED Vermelho

**Fluxo de Comunicação entre Tarefas**
- **Tarefa 1 → Event Group**: Envia sinais de mudança de estado do botão
- **Event Group → Tarefa 2**: Sinaliza botão pressionado para mudança de cor
- **Event Group → Tarefa 3**: Sinaliza estado do botão (ON/OFF)
- **Tarefa 2 → Queue**: Envia string com nome da cor atual
- **Queue → Tarefa 4**: Recebe string da cor para exibição
- **Mutex**: Protege acesso simultâneo ao OLED (Tarefas 3 e 4)

## 🗺️ Fluxo de Controle Principal (Pseudo-código)

```pseudocode
🔧 INICIAR:
    Configurar I2C para display OLED
    Configurar GPIOs para LED RGB (Vermelho, Verde, Azul)
    Configurar botão GPIO 5 com pull-up interno
    Criar Event Group (3 bits)
    Criar Queue (5 elementos, 10 bytes cada)
    Criar Mutex para proteção do OLED
    Inicializar display OLED
    Acender LED vermelho
    Escrever "BOTAO OFF" na primeira linha do OLED
    Escrever "VERMELHO" na segunda linha do OLED
    Criar 4 tarefas FreeRTOS

🔄 ENQUANTO sistema_ativo:

    📌 TAREFA 1 (Leitura do Botão):
        a) Ler estado atual do botão
        b) Comparar com estado anterior
        c) SE estado mudou:
           - Se mudou para ON: Setar BUTTON_ON_BIT_1 e BUTTON_ON_BIT_2 no Event Group
           - Se mudou para OFF: Setar BUTTON_OFF_BIT no Event Group
        d) Atualizar estado anterior
        e) Aguardar 50ms (vTaskDelay com pdMS_TO_TICKS(50))
    
    📌 TAREFA 2 (Controle do LED RGB):
        a) Aguardar BUTTON_ON_BIT_1 no Event Group
        b) Limpar BUTTON_ON_BIT_1 após receber
        c) Alternar cor do LED (Vermelho → Verde → Azul → Vermelho)
        d) Acender LED na nova cor
        e) Copiar nome da cor para string (strcpy)
        f) Enviar string pela Queue (timeout 100ms)
    
    📌 TAREFA 3 (Estado do Botão no OLED):
        a) Aguardar BUTTON_ON_BIT_2 ou BUTTON_OFF_BIT no Event Group
        b) Guardar bits recebidos antes da limpeza
        c) Pegar Mutex
        d) Determinar texto baseado nos bits:
           - Se BUTTON_ON_BIT_2: "BOTAO ON"
           - Se BUTTON_OFF_BIT: "BOTAO OFF"
        e) Limpar primeira linha do OLED
        f) Escrever texto na primeira linha
        g) Atualizar display
        h) Liberar Mutex
    
    📌 TAREFA 4 (Cor no OLED):
        a) Aguardar string da Queue (timeout infinito)
        b) Pegar Mutex
        c) Limpar segunda linha do OLED
        d) Escrever string recebida na segunda linha
        e) Atualizar display
        f) Liberar Mutex

🏁 FIM:
    Sistema continua rodando indefinidamente
```

## Implementação Física de _Hardware_ _Off-line_

## Interface com Mundo Físico

### Entradas

- **Botão (GPIO 5)**: Botão físico para controlar mudança de cor do LED RGB
  - Pressionar para alternar cor do LED (vermelho → verde → azul)
  - **Pull-up interno** ativado para evitar flutuações
  - **Leitura contínua** pela Tarefa 1 com detecção de mudança de estado
  - **Delay de 50ms** entre leituras para evitar ocupação excessiva do processador

### Saídas

- **Display OLED SSD1306 (I2C - GPIO 2/3)**: Interface visual principal
  - **Primeira linha**: Exibe estado do botão ("BOTAO ON" ou "BOTAO OFF")
  - **Segunda linha**: Exibe cor atual do LED ("VERMELHO", "VERDE" ou "AZUL")
  - Atualização em tempo real conforme interações
  - **Acesso protegido** por Mutex entre Tarefas 3 e 4
  - Resolução 128x64 pixels

- **LED RGB (GPIO 11, 12, 13)**: Feedback visual da cor atual
  - **GPIO 13**: LED Vermelho (cor inicial)
  - **GPIO 11**: LED Verde
  - **GPIO 12**: LED Azul
  - Sequência de alternância: Vermelho → Verde → Azul → Vermelho
  - Controle pela Tarefa 2 após detecção de botão pressionado

### Guia de Uso

#### 🚀 Inicialização do Sistema

1. **Ligue o sistema**: Conecte a alimentação USB à Raspberry Pi Pico
2. **Aguarde inicialização**: O display mostrará "BOTAO OFF" e "VERMELHO"
3. **Verifique LED**: O LED RGB deve estar aceso na cor vermelha
4. **Sistema pronto**: Todas as 4 tarefas FreeRTOS estarão ativas

#### 🎨 Controlando o LED RGB

1. **Pressione o botão**: O sistema detectará a mudança de estado (OFF→ON)
2. **Aguarde a transição**: A cor do LED mudará automaticamente
   - Primeiro pressionamento: Vermelho → Verde
   - Segundo pressionamento: Verde → Azul
   - Terceiro pressionamento: Azul → Vermelho
3. **Observe o display**: A segunda linha será atualizada com o nome da cor
4. **Soltar o botão**: A primeira linha mostrará "BOTAO OFF"


## 🚀 Instalação e Configuração

### Pré-requisitos

- **Visual Studio Code** com as seguintes extensões:
  - **CMake Tools** (Microsoft)
  - **Raspberry Pi Pico**
  - **Cortex-Debug** (marus25)
- **DAP Link** para carregamento (_flashing_) e depuração (_debugging_) (opcional)
- **Raspberry Pi Pico SDK** instalado
- [**FreeRTOS Kernel**](https://github.com/FreeRTOS/FreeRTOS.git): incluído no projeto em `FreeRTOS-Kernel/`
- [**Biblioteca SSD1306**](https://github.com/daschr/pico-ssd1306): Driver para display [`OLED SSD1306`](include)
  - Inclui: `ssd1306.h`, `ssd1306_i2c.h`, `ssd1306_i2c.c`, `ssd1306_font.h`

### 1. Clonar o Repositório

```bash
git clone <URL_DO_REPOSITORIO>
cd P4
```

### 2. Configurar e Executar no VSCode

1. **Abrir o projeto no VSCode**:
   ```bash
   code .
   ```

2. **Importar o projeto na extensão Raspberry Pi Pico**:
   - Vá para a extensão "Raspberry Pi Pico" no VSCode
   - Clique em "Import Project" ou "Importar Projeto"
   - Selecione a pasta do projeto clonado

3. **Conectar o _hardware_**:
   - Conecte o display OLED aos pinos GPIO 2 (SDA) e GPIO 3 (SCL)
   - Conecte o LED RGB: GPIO 13 (Vermelho), GPIO 11 (Verde), GPIO 12 (Azul)
   - Conecte o botão ao GPIO 5
   - Conecte o DAP Link aos pinos DEBUG da Raspberry Pi Pico e a uma porta USB do computador (opcional para debug)
   - Certifique-se de que a Pico está conectada via mini-USB a outra porta USB do computador

4. **Compilar e Debuggar**:
   - Na extensão Raspberry Pi Pico, clique em "Debug" ou "Debugar"
   - A extensão automaticamente:
     - Compila o projeto
     - Faz o flashing para a Pico
     - Inicia a sessão de debugging
   - Use breakpoints e inspeção de variáveis conforme necessário
   - Configure breakpoints em cada tarefa para verificar execução

### 3. Instalação Simples (Para Usuários Finais)

Se você apenas quer usar o sistema sem desenvolver ou depurar:

1. **Baixar o firmware**:
   - Acesse a pasta `build/` do projeto
   - Localize o arquivo `main.uf2`

2. **Carregar na Raspberry Pi Pico**:
   - Mantenha pressionado o botão BOOTSEL da Pico
   - Conecte a Pico via USB ao computador (mantendo BOOTSEL pressionado)
   - Solte o botão BOOTSEL - a Pico aparecerá como unidade USB "RPI-RP2"
   - Copie o arquivo `main.uf2` para a unidade RPI-RP2
   - A Pico reiniciará automaticamente e executará o firmware

3. **Conectar o hardware**:
   - Conecte o display OLED, LED RGB e botão conforme especificado
   - O sistema estará pronto para uso imediatamente

## 🧪 Testes de Validação

Foram conduzidos os seguintes procedimentos de teste para validar o funcionamento do sistema:

### **Teste das Tarefas FreeRTOS**
- Verificação da criação e inicialização das 4 tarefas
- Validação da execução concorrente e trocas de contexto
- Teste de prioridades e escalonamento de tarefas

### **Teste do Event Group**
- Validação do set de bits quando botão é pressionado (BUTTON_ON_BIT_1 e BUTTON_ON_BIT_2)
- Validação do set de bit quando botão é solto (BUTTON_OFF_BIT)
- Teste de limpeza de bits após leitura por cada tarefa
- Verificação de que duas tarefas recebem o sinal de botão pressionado corretamente

### **Teste da Queue**
- Verificação do envio de strings de cor pela Tarefa 2
- Validação da recepção pela Tarefa 4
- Teste com timeout de 100ms no envio
- Validação de strings completas ("VERMELHO", "VERDE", "AZUL")

### **Teste do Mutex**
- Verificação de acesso exclusivo ao display OLED
- Teste de proteção contra acesso simultâneo das Tarefas 3 e 4
- Validação de que não há corrupção de dados no display

### **Teste do LED RGB**
- Verificação da sequência de cores: Vermelho → Verde → Azul
- Teste de cada cor individualmente (GPIO 11, 12, 13)
- Validação do estado inicial (vermelho aceso)

### **Teste do Display OLED**
- Verificação da inicialização e exibição do estado inicial
- Teste de atualização da primeira linha ("BOTAO ON" / "BOTAO OFF")
- Teste de atualização da segunda linha (nome da cor)
- Validação da limpeza de linhas antes de reescrita

### **Teste do Botão**
- Verificação da detecção de mudança de estado (OFF→ON e ON→OFF)
- Validação do pull-up interno
- Teste do delay de 50ms entre leituras
- Validação de que apenas mudanças de estado geram eventos

Todos os testes foram concluídos com resultado satisfatório

## 🤝 Autores

Este projeto foi desenvolvido por **Beatriz Moreira** e **Gustavo de Melo**   
como parte da disciplina **EA801** (turma A) na **FEEC** da **Unicamp**.

## 📄 Licença

Este projeto é parte de um trabalho acadêmico da disciplina EA801 - 
Sistemas Embarcados da Unicamp.

---

**FreeRTOS Desafio na sua Raspberry Pi Pico!** 🔴🟢🔵✨
