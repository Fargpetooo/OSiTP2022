// Подключение библиотек
#include "thread.h"
#include "periph/gpio.h"
#include "xtimer.h"
#include "msg.h"

//Количество микросекунд, прошедших с последнего вызова обработчика
uint32_t timer = 0;
uint32_t count = 0;

// Выделение памяти под стеки двух тредов
char thread_one_stack[THREAD_STACKSIZE_DEFAULT];
char thread_two_stack[THREAD_STACKSIZE_DEFAULT];

// Глобально объявляется структура, которая будет хранить PID (идентификатор треда)
// Нам нужно знать этот идентификатор, чтобы посылать сообщения в тред
static kernel_pid_t thread_one_pid;

// Обработчик прерывания с кнопки
void btn_handler(void *arg)
{
// Прием аргументов из главного потока
(void)arg;
// Создание объекта для хранения сообщения


if ((xtimer_usec_from_ticks(xtimer_now()) - timer) > 250000){
if (gpio_read(GPIO_PIN(PORT_A,0)) > 0){
msg_t msg;
count++;
if (count > 5) {
count = 1;
}  
msg.content.value = count;
msg_send(&msg, thread_one_pid);
timer = xtimer_usec_from_ticks(xtimer_now());
}
}
}

// Первый поток
void *thread_one(void *arg)
{
// Прием аргументов из главного потока
(void) arg;
// Создание объекта для хранения сообщения
msg_t msg;
// Инициализация пина PC8 на выход
gpio_init(GPIO_PIN(PORT_C,8),GPIO_OUT);
while(1){
// Ждем, пока придет сообщение
// msg_receive - это блокирующая операция, поэтому задача зависнет в этом месте, пока не придет сообщение
msg_receive(&msg);
// Переключаем светодиод

for (uint32_t i = 1; i <= msg.content.value; i++) {
gpio_set(GPIO_PIN(PORT_C,8));
xtimer_usleep(500000);
gpio_clear(GPIO_PIN(PORT_C,8));
xtimer_usleep(500000);
}

//xtimer_usleep(5000);
}
// Хотя код до сюда не должен дойти, написать возврат NULL все же обязательно надо
return NULL;
}

// Второй поток
void *thread_two(void *arg)
{

// Прием аргументов из главного потока
(void) arg;
// Инициализация пина PC9 на выход
gpio_init(GPIO_PIN(PORT_C,9),GPIO_OUT);

while(1){
// Включаем светодиод
gpio_set(GPIO_PIN(PORT_C,9));
// Задача засыпает на 333333 микросекунды
xtimer_usleep(333333);
// Выключаем светодиод
gpio_clear(GPIO_PIN(PORT_C,9));
// Задача снова засыпает на 333333 микросекунды
xtimer_usleep(333333);
}
return NULL;
}

int main(void)
{
// Инициализация прерывания с кнопки (подробнее в примере 02button)
gpio_init_int(GPIO_PIN(PORT_A,0),GPIO_IN,GPIO_RISING,btn_handler,NULL);

// Создение потоков (подробнее в примере 03threads)
// Для первого потока мы сохраняем себе то, что возвращает функция thread_create,
// чтобы потом пользоваться этим как идентификатором потока для отправки ему сообщений
thread_one_pid = thread_create(thread_one_stack, sizeof(thread_one_stack),
THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST,
thread_one, NULL, "thread_one");
// обратите внимание, что у потоков разный приоритет: на 1 и на 2 выше, чем у main
thread_create(thread_two_stack, sizeof(thread_two_stack),
THREAD_PRIORITY_MAIN - 2, THREAD_CREATE_STACKTEST,
thread_two, NULL, "thread_two");

while (1){

}

return 0;
}

/*
Задание 1: Добавьте в код подавление дребезга кнопки
Задание 2: Сделайте так, чтобы из прерывания по нажатию кнопки в поток thread_one передавалось целое число,
которое означает, сколько раз должен моргнуть светодиод на пине PC8 после нажатия кнопки.
После каждого нажатия циклически инкрементируйте значение от 1 до 5.
Передать значение в сообщении можно через поле msg.content.value
Задание 3: Сделайте так, чтобы из прерывания по отпусканию кнопки в поток thread_two передавалось целое число,
которое задает значение интервала между морганиями светодиода на пине PC9.
Моргание светодиода не должно останавливаться.
После каждого нажатия циклически декрементируйте значение от 1000000 до 100000 с шагом 100000.
Чтобы послать сообщение асинхронно и без блокирования принимающего потока, нужно воспользоваться очередью.
Под очередь нужно выделить память в начале программы так: static msg_t rcv_queue[8];
Затем в принимающем потоке нужно ее инициализировать так: msg_init_queue(rcv_queue, 8);
Поток может проверять, лежит ли что-то в очереди, это делается функцией msg_avail(),
которая возвращает количество*/
