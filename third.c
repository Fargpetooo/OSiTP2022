// Подключение библиотек
#include "thread.h"
#include "xtimer.h"
#include "timex.h"
#include "periph/gpio.h"

// Выделение памяти под стек первого треда
// Размер выделяемого стека задан макросом THREAD_STACKSIZE_DEFAULT
char stack_one[THREAD_STACKSIZE_DEFAULT];
char stack_two[THREAD_STACKSIZE_DEFAULT];

// Это первый поток
void *thread_one(void *arg)
{
    // Прием аргументов из главного потока
    (void) arg;
    // ВременнАя метка для отсчета времени сна
    xtimer_ticks32_t last_wakeup_one = xtimer_now();
    while(1){
        // Переключение состояния пина PC8
    	gpio_toggle(GPIO_PIN(PORT_C,8));
        // Поток засыпает на 500000 микросекунд
    	xtimer_periodic_wakeup(&last_wakeup_one, 500000);
    }
    return NULL;
}

// Это второй поток
void *thread_two(void *arg)
{
    // Прием аргументов из главного потока
    (void) arg;
    // ВременнАя метка для отсчета времени сна
    xtimer_ticks32_t last_wakeup_two = xtimer_now();
    while(1){
        // Переключение состояния пина PC9
        gpio_toggle(GPIO_PIN(PORT_C,9));
        // Поток засыпает на 1000000 микросекунд
        xtimer_periodic_wakeup(&last_wakeup_two, 1000000);
    }
    return NULL;
}


int main(void)
{
    // Инициализация пина PC8 на выход
	gpio_init(GPIO_PIN(PORT_C,8), GPIO_OUT);
	gpio_init(GPIO_PIN(PORT_C,9), GPIO_OUT);

    // Создание потока
    // stack_one - выделенная под стек память
    // sizeof(stack_one) - размер стека
    // THREAD_PRIORITY_MAIN-1 - приоритет потока на 1 больше, чем у потока main.
    // THREAD_CREATE_STACKTEST - флаги, которые создают маркеры использования стека
    // thread_one - имя потока
    // NULL - передаваемые в поток аргументы
    // "thread_one" - дескриптор
    thread_create(stack_one, sizeof(stack_one),
                    THREAD_PRIORITY_MAIN-1,
                    THREAD_CREATE_STACKTEST,
                    thread_one,
                    NULL, "thread_one");
    thread_create(stack_two, sizeof(stack_two),
                    THREAD_PRIORITY_MAIN-1,
                    THREAD_CREATE_STACKTEST,
                    thread_two,
                    NULL, "thread_two");

    return 0;
}

/*
    Задание 1. Создайте второй поток, который будет моргать другим светодиодом с другой частотой. 
                Обратите внимание, что не рекомендуется создавать потоки с одинаковым приоритетом!
*/
