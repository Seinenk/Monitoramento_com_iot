/*
 *  Pico W – Monitor de Joystick + LED + Botões + Temperatura do processador
 *  Autor : Carlos Michael da Silva Sousa
 *  Data  : mai/2025
 *
 *  Funciona com a placa BitDogLab.
 *  - Conecta em Wi-Fi (dados em hw_config.h)
 *  - Alterna LED vermelho (GPIO 13) a cada envio
 *  - Lê joystick nos ADC-0 (Y) e ADC-1 (X)
 *  - Lê estado dos botões
 *  - Lê a temperatura atual do processador
 *  - Constrói JSON e faz POST
 *  - Pronto para enviar para o repositório
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hardware/adc.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "lwip/ip4_addr.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include "hw_config.h" /* SSID, pinos, LED, servidor */

/* ------------------------------------------------------------------ */
/* AUXÍLIO A LOG & ERRO                                               */
/* ------------------------------------------------------------------ */
static inline void check(bool ok, const char *msg)
{
    if (!ok)
    {
        printf("ERRO: %s\n", msg);
        while (1)
            tight_loop_contents(); /* trava */
    }
}

/* ------------------------------------------------------------------ */
/* WIFI – conecta em modo estação                                     */
/* ------------------------------------------------------------------ */
static void wifi_conectar(void)
{
    check(cyw43_arch_init() == 0, "cyw43 init");
    cyw43_arch_enable_sta_mode();

    printf("Wi-Fi: tentando conectar em \"%s\"…\n", WIFI_SSID);

    int rc = cyw43_arch_wifi_connect_timeout_ms(
        WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 20000);

    check(rc == 0, "falha Wi-Fi");

    printf("Wi-Fi OK  – IP %s\n",
           ip4addr_ntoa(netif_ip4_addr(netif_default)));
}

/* ------------------------------------------------------------------ */
/* JOYSTICK                                                           */
/* ------------------------------------------------------------------ */
typedef struct
{
    float x_norm;        /* -1 … +1 */
    float y_norm;        /* -1 … +1 */
    const char *direcao; /* “north”, “south”… */
} InfoJoy;

/* Converte par (x,y) normalizado em rosa-dos-ventos */
static const char *rosa(float x, float y)
{
    if (fabsf(x) < 0.2f && fabsf(y) < 0.2f)
        return "none";

    float ang = atan2f(y, x) * 180.f / (float)M_PI; /* -180 … 180 */
    if (ang < -157.5f)
        return "Oeste";
    if (ang < -112.5f)
        return "Sudoeste";
    if (ang < -67.5f)
        return "Sul";
    if (ang < -22.5f)
        return "Sudeste";
    if (ang < 22.5f)
        return "Leste";
    if (ang < 67.5f)
        return "Nordeste";
    if (ang < 112.5f)
        return "Norte";
    if (ang < 157.5f)
        return "Noroeste";
    return "Oeste";
}

static inline void buttons_init(void)
{
    gpio_init(BTN1_PIN);
    gpio_set_dir(BTN1_PIN, GPIO_IN);
    gpio_pull_up(BTN1_PIN); // entrada com pull-up

    gpio_init(BTN2_PIN);
    gpio_set_dir(BTN2_PIN, GPIO_IN);
    gpio_pull_up(BTN2_PIN);
}

static inline void buttons_get_json(char *buf, size_t len)
{
    int b1 = !gpio_get(BTN1_PIN); // invertido por causa do pull-up
    int b2 = !gpio_get(BTN2_PIN);
    snprintf(buf, len, "{ \"btn1\": %d, \"btn2\": %d }", b1, b2);
}

static void joystick_init(void)
{
    adc_init();
    adc_gpio_init(26); /* ADC0  -> Y */
    adc_gpio_init(27); /* ADC1  -> X */
}

static InfoJoy joystick_ler(void)
{
    const float MAX = (1 << 12) - 1;
    const float MEIO = MAX / 2.f;

    adc_select_input(1); /* X */
    float x = adc_read();
    adc_select_input(0); /* Y */
    float y = adc_read();

    InfoJoy out = {
        .x_norm = (x - MEIO) / MEIO, /* -1…+1  */
        .y_norm = (y - MEIO) / MEIO};
    out.direcao = rosa(out.x_norm, out.y_norm);
    return out;
}

/* ------------------------------------------------------------------ */
/* ENVIO HTTP                                                         */
/* ------------------------------------------------------------------ */
static err_t tcp_finish(void *arg,
                        struct tcp_pcb *tpcb,
                        struct pbuf *p,
                        err_t err)
{
    if (p)
    {
        /* avisa ao stack que consumimos os bytes */
        tcp_recved(tpcb, p->tot_len);
        pbuf_free(p);
    }

    tcp_close(tpcb); /* encerra a sessão TCP                      */
    return ERR_OK;
}

static void enviar_json(const char *json, const char *endpt)
{
    struct tcp_pcb *pcb = tcp_new();
    ip_addr_t dst;
    ipaddr_aton(SERVER_IP, &dst);
    if (tcp_connect(pcb, &dst, SERVER_PORT, NULL) != ERR_OK)
        return;

    char req[512];
    snprintf(req, sizeof(req),
             "POST %s HTTP/1.1\r\n"
             "Host: %s:%d\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %d\r\n\r\n"
             "%s",
             endpt, SERVER_IP, SERVER_PORT, (int)strlen(json), json);

    tcp_write(pcb, req, strlen(req), TCP_WRITE_FLAG_COPY);
    tcp_output(pcb);
    tcp_recv(pcb, tcp_finish);
}

static void post_buttons_info(void)
{
    char json[64];
    buttons_get_json(json, sizeof json);
    printf("JSON botões: %s\n", json);
    enviar_json(json, "/botoes");
}

/* ------------------------------------------------------------------ */
/* TEMPERATURA INTERNA (ADC-4)                                        */
/* ------------------------------------------------------------------ */
static float read_cpu_temp(void)
{
    adc_select_input(4);       // ADC-4 = sensor interno
    uint16_t raw = adc_read(); // leitura bruta 0-4095

    const float VREF = 3.3f;            // tensão de referência
    const float ADC_CONV = VREF / 4096; // fator de conversão
    float v = raw * ADC_CONV;           // tensão em volts

    // Fórmula do datasheet – ±2 °C
    return 27.0f - (v - 0.706f) / 0.001721f;
}

static void post_cpu_temp(void)
{
    float t = read_cpu_temp();
    char json[64];
    snprintf(json, sizeof json, "{ \"temp\": %.2f }", t);
    printf("JSON temperatura: %s\n", json);
    enviar_json(json, "/temp");
}

/* ------------------------------------------------------------------ */
/* LED                                                                */
/* ------------------------------------------------------------------ */
static void led_config(void)
{
    gpio_init(LED_RED_PIN);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);
}

static void led_toggle(void)
{
    static bool state = false;
    gpio_put(LED_RED_PIN, state);
    state = !state;
}

/* ------------------------------------------------------------------ */
/* LOOP PRINCIPAL                                                     */
/* ------------------------------------------------------------------ */
int main(void)
{
    stdio_init_all();
    sleep_ms(2500); /* USB enum                */

    led_config();
    joystick_init();
    wifi_conectar();
    buttons_init();
    adc_set_temp_sensor_enabled(true); // habilita sensor interno

    absolute_time_t t0 = get_absolute_time();

    while (true)
    {
        cyw43_arch_poll(); /* mantém Wi-Fi vivo */

        /* cronômetro de 1 s ------------------------------------ */
        if (absolute_time_diff_us(t0, get_absolute_time()) >= 1e6)
        {
            t0 = get_absolute_time();

            /* LED “batimento cardíaco” ------------------------- */
            led_toggle();

            /* ➋  JOYSTICK  ------------------------------------ */
            InfoJoy j = joystick_ler();
            char json[160];
            snprintf(json, sizeof json,
                     "{ \"x\": %.3f, \"y\": %.3f, \"rosa\": \"%s\" }",
                     j.x_norm, j.y_norm, j.direcao);
            enviar_json(json, "/joystick");

            /* ➊  BOTÕES  -------------------------------------- */
            post_buttons_info();

            /* ➌  TEMPERATURA ------------------------------------------------ */
            post_cpu_temp();

            printf("JSON joystick: %s\n", json);
        }
    }
}
