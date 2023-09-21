/*
----------------------------------
| 3E Unicamp ~ RFID Interpreter |
---------------------------------
O intuito deste projeto é simples:
Utilizando o ESP32 queremos criar um código que seja capaz de realizar a leitura do valor
armazenado dentro do TAG do RFID e 

Antes de começar a explicação, de cada trecho de código, preciso comentar que programar em ESP32 de uma
forma mais profissional, é BASTANTE diferente à programação com o Arduino framework. A principal diferença
que irá se notar é sobre o sequênciamento do nosso código, e sobre nosso entendimento da ordem de sua
execução. O ESP32 na framework do ESPIDF funciona em termos de "threads" e em termos de "eventos".
Por conta disso, preciso explicar alguns detalhes especiais do nosso código:

- No ESPIDF nós programamos usando freeRTOS, uma ferramenta que estrutura o nosso código de uma forma
  BEM diferente ao Arduino. 
  Vocês alguma vez se perguntaram como o nosso PC faz para executar tantas tarefas de forma simultânea? 
  Ele, na real, não executa as tarefas de forma totalmente simultânea. O que ele faz é que ele executa
  vários "threads" ou "tasks" de forma simultânea, com um tempo bem pequeno entre cada execução.
  Pensem em pequenas tarefas de curta duração sendo executadas com um tempo MUITO curto entre cada. Se o tempo
  for curto o suficiente, a execução das tarefas vai parecer continua, não é?
  O freeRTOS pega este principio de funcionamento, e consegue trazer ele ao mundo dos microcontroladores.
  Essencialmente, estamos definindo vários threads que serão executados no nosso programa.
  
- Notemos que múltiplas variáveis que estamos usando nele foram definidas como sendo static, por que?
  Lembrando que estamos usando freeRTOS, existem certas variáveis que precisarão existir ao longo da execução
  de TODOS os threads do nosso programa. Ao definir uma variável como static, estamos deixando claro ao compilador
  que nossa variável deverá existir sempre.
*/

// Defino as bibliotecas que irei utilizar neste projeto.
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <esp_log.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rc522.h"

/*
Como estarei usando registros de erros nas bibliotecas usadas neste projeto, preciso definir
um TAG para entender os erros que aconteçam na execução do main.c .
*/
static const char* TAG = "esp32rfid";

/*
Aqui defino uma variável conhecida como scanner, que é um ponteiro à estrutura rc522 definida
em rc522.c. Ela será bastante importante ao longo da execução do meu programa, pois é mediante ela que eu
vou estar brincando e definindo certos paramétros importantes no controle do periférico rc522.
*/
static rc522_handle_t scanner;

/*
Neste bloco de código eu crio o handler que vai estar lidando com a recepção de informações por parte
do periférico e meus pinos configurados para uso como SPI.
*/
static void rc522_handler(void* arg, esp_event_base_t base, int32_t event_id, void* event_data)
{
    rc522_event_data_t* data = (rc522_event_data_t*) event_data;

    switch(event_id) {
        case RC522_EVENT_TAG_SCANNED: {
                rc522_tag_t* tag = (rc522_tag_t*) data->ptr;
                ESP_LOGI(TAG, "Tag scanned (sn: %" PRIu64 ")", tag->serial_number);
            }
            break;
    }
}

void app_main() {
    /*
    Aqui eu defino os pinos que irei utilizar no meu ESP32 para realizar a comunicação com o módulo
    mediante SPI (Serial Peripheral Interface), além de inicializar ele.

    O que o programa está fazendo, é que ele está accesando o struct definido como rc522_config_t e, nos
    dados do struct interno .spi, está colocando as informações requisitadas pela biblioteca.
    É possível ver o seu funcionamento com maior detalhe ao abrir o rc522.h .


    */
    rc522_config_t config = {
        .spi.host = VSPI_HOST, // Este aqui é o spi bus que estaremos utilizando.
        .spi.miso_gpio = 25,
        .spi.mosi_gpio = 23,
        .spi.sck_gpio = 19,
        .spi.sda_gpio = 22,
    };

    /*
    Nesta primeira parte eu crio um evento associado ao rc522 seguindo a sua biblioteca.
    */
    rc522_create(&config, &scanner);
    /*
    Aqui eu defino que para qualquer evento com o RC522, o handler definido no começo do programa
    será executado.
    */
    rc522_register_events(scanner, RC522_EVENT_ANY, rc522_handler, NULL);
    /*
    Aqui eu inicializo o funcionamento do meu rc522 usando o espaço de memória alocado em rc522_create
    para scanner, como um pseudo-registrador.
    */
    rc522_start(scanner);   
}




