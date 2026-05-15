#include <LiquidCrystal.h>

//declarando os botões
const int pinoBotao1 = 10;
const int pinoBotao2 = 9;
const int pinoBotao3 = 8;
const int pinoBotaoInicial = 13;
const int pinoEmergencia = 2;
volatile bool modoEmergencia = false;

// declarando led e buzzer

const int ledVermelho = 7;
const int ledAmarelo = 6;
const int ledVerde = 1; 
const int pinoBuzzer = A4; 
const int pinoPotenciometro = A5;

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 0;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Lógica de Senha
const int sequenciaCorreta[3] = {3, 1, 2}; // armazenando sequência correta
int sequenciaSelecionada[3] = {0, 0, 0};
int indiceSenha = 0;       // Controla quantos números já foram digitados
bool aguardandoSenha = false; // Estado do sistema

void setup() {
  // componentes de entrada
  pinMode(pinoBotao1, INPUT_PULLUP); 
  pinMode(pinoBotao2, INPUT_PULLUP); 
  pinMode(pinoBotao3, INPUT_PULLUP);
  pinMode(pinoBotaoInicial, INPUT_PULLUP);
  pinMode(pinoEmergencia, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(pinoEmergencia), ativarEmergencia, FALLING);
  
  // componentes de saída
  pinMode(ledVermelho, OUTPUT);
  pinMode(ledAmarelo, OUTPUT);
  pinMode(ledVerde, OUTPUT);
  pinMode(pinoBuzzer, OUTPUT);

  lcd.begin(16,2); // quantidade de colunas e linhas do lcd
  digitalWrite(ledVermelho, HIGH); // Estado inicial
}

void loop() {
  
  // Se a interrupção foi acionada, executa o bloqueio
  if (modoEmergencia) {
    // Zera o progresso do usuário
    aguardandoSenha = false; 
    indiceSenha = 0;
    
    // Configura os LEDs para estado de alerta
    digitalWrite(ledAmarelo, LOW);
    digitalWrite(ledVerde, LOW);
    digitalWrite(ledVermelho, HIGH); 
    
    // Atualiza a interface
    lcd.clear();
    lcd.print("EMERGENCIA!");
    lcd.setCursor(0,1);
    lcd.print("SISTEMA TRAVADO");
    
    tone(pinoBuzzer, 1500, 1000); // Som de alarme
    delay(5000); // Mantém o cofre travado em alerta por 5 segundos
    
    lcd.clear();
    modoEmergencia = false; // Desativa o alerta para o sistema voltar ao repouso normal
  }
  // Leitura dos botões (LOW significa que foi pressionado)
  bool b1 = (digitalRead(pinoBotao1) == LOW);
  bool b2 = (digitalRead(pinoBotao2) == LOW);
  bool b3 = (digitalRead(pinoBotao3) == LOW);
  bool bInicial = (digitalRead(pinoBotaoInicial) == LOW);

  // ESTADO 1: Sistema travado, esperando apertar o botão inicial pela primeira vez
  if (!aguardandoSenha) {
    if (bInicial) {
      aguardandoSenha = true;
      indiceSenha = 0; // Reseta a contagem da senha
      
      //apenas o led vermelho aceso
      digitalWrite(ledVermelho, LOW);
      digitalWrite(ledVerde, LOW);
      digitalWrite(ledAmarelo, HIGH);
      
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Digite a senha:");
      delay(300); // Pequeno atraso para evitar duplo clique
    }
  } 
  // ESTADO 2: Sistema aguardando os 3 dígitos e a confirmação
  else {
    // Capturando os botões digitados 
    if (b1 && indiceSenha < 3) { registrarBotao(1); }
    if (b2 && indiceSenha < 3) { registrarBotao(2); }
    if (b3 && indiceSenha < 3) { registrarBotao(3); }

    // Apertar o botao inicial de novo para confirmar a senha
    if (bInicial) {
      if (indiceSenha == 3) {
        verificarSenha(); // Só verifica se já digitou os 3
      } else {
        lcd.setCursor(0,1);
        lcd.print("Faltam digitos!");
        delay(1000);
        lcd.setCursor(0,1);
        lcd.print("                "); // Limpa a mensagem
      }
      delay(300); 
  }
}
  }

// Função para salvar o botão apertado e mostrar no LCD
void registrarBotao(int numBotao) {
  sequenciaSelecionada[indiceSenha] = numBotao;
  
  lcd.setCursor(indiceSenha, 1);
  lcd.print("*"); // Mostra um asterisco para cada botão apertado (como senha real)
  
  indiceSenha++;
  delay(300); // Delay para não registrar o mesmo clique duas vezes
}

// Função que compara as listas e aciona os LEDs/Som
void verificarSenha() {
  bool senhaCorreta = true;
  
  // Compara a sequenciaSelecionada com a sequenciaCorreta
  for (int i = 0; i < 3; i++) {
    if (sequenciaSelecionada[i] != sequenciaCorreta[i]) {
      senhaCorreta = false;
      break;
    }
  }

  // --- LÓGICA DO POTENCIÔMETRO ---
  int valorPotenciometro = analogRead(pinoPotenciometro);
  bool posicaoCorreta = false;
  
  // Usei uma margem de segurança (450 a 550)
  if (valorPotenciometro >= 450 && valorPotenciometro <= 550) {
    posicaoCorreta = true;
  }

  lcd.clear();
  digitalWrite(ledAmarelo, LOW); // Desliga amarelo em ambos os casos

  // O cofre SÓ abre se a senha estiver certa E o potenciômetro no meio
  if (senhaCorreta && posicaoCorreta) {
    digitalWrite(ledVerde, HIGH);
    lcd.print("Cofre Aberto!");
    tone(pinoBuzzer, 1000, 500); // Som de sucesso
    
  } else {
    digitalWrite(ledVermelho, HIGH);
    
    // Mensagens diferentes para você saber o que o usuário errou
    if (!senhaCorreta) {
      lcd.print("Senha Incorreta!");
    } else {
      lcd.print("Posicao Invalida"); // A senha estava certa, mas o potenciômetro estava errado
    }
    tone(pinoBuzzer, 200, 500); // Som de erro
  }

  // Finaliza a verificação e volta para o estado inicial após 5 segundos
  delay(5000); 
  lcd.clear();
  aguardandoSenha = false; 
  
  // Mantém a trava se errou alguma coisa, ou reseta o sistema se acertou
  if (!(senhaCorreta && posicaoCorreta)) {
    digitalWrite(ledVermelho, HIGH);
  } else {
    digitalWrite(ledVerde, LOW);
    digitalWrite(ledVermelho, HIGH); // Reseta o sistema travando de novo
  }
}
void ativarEmergencia() {
  modoEmergencia = true;
}