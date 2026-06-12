#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>

// --- log ---
LOG_MODULE_REGISTER(app, LOG_LEVEL_DBG);
// ---

volatile int8_t count = 0;
K_MUTEX_DEFINE(count_mutex);

// --- add ---
void add_thread_entry(void *arg0, void *arg1, void *arg2)
{
    while(1)
    {
        k_mutex_lock(&count_mutex, K_FOREVER);
        count++;
        LOG_DBG("[%s] count: %d",  k_thread_name_get(k_current_get()), count);
        k_mutex_unlock(&count_mutex);

        k_sleep(K_MSEC(1000));
    }
}
// ---

// --- sub ---
void sub_thread_entry(void *arg0, void *arg1, void *arg2)
{
    while(1)
    {
        k_mutex_lock(&count_mutex, K_FOREVER);
        count--;
        LOG_DBG("[%s] count: %d",  k_thread_name_get(k_current_get()), count);
        k_mutex_unlock(&count_mutex);

        k_sleep(K_MSEC(1500));
    }
}

// --- threads ---
#define STACK_SIZE 512

#define PRIORITY 1

K_THREAD_DEFINE(add_id, STACK_SIZE, add_thread_entry, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(sub_id, STACK_SIZE, sub_thread_entry, NULL, NULL, NULL, PRIORITY, 0, 0);
// ---

int main()
{
    // --- threads ---
    k_thread_name_set(add_id, "add");
    k_thread_name_set(sub_id, "sub");
    // ---

    while (1) 
    {
        k_sleep(K_FOREVER);
    }

    return 0;
}

