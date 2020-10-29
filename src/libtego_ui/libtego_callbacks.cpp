#include "utils/Settings.h"

namespace
{
    // data
    std::unique_ptr<QTimer> taskTimer;
    std::vector<std::function<void()>> taskQueue;
    std::mutex taskQueueLock;

    void consume_tasks()
    {
        // get sole access to the task queue
        static decltype(taskQueue) localTaskQueue;
        {
            std::lock_guard<std::mutex> lock(taskQueueLock);
            std::swap(taskQueue, localTaskQueue);
        }

        // consume all of our tasks
        for(auto task : localTaskQueue)
        {
            try
            {
                task();
            }
            catch(std::exception& ex)
            {
                qDebug() << "Exception thrown from task: " << ex.what();
            }
        }

        // clear out our queue
        localTaskQueue.clear();
    }

    template<typename FUNC>
    void push_task(FUNC&& func)
    {
        // acquire lock on the queue and push our received functor
        std::lock_guard<std::mutex> lock(taskQueueLock);
        taskQueue.push_back(std::move(func));
    }

    //
    // libtego callbacks
    //

    void on_new_identity_created(
        tego_context_t*,
        const tego_ed25519_private_key_t* privateKey)
    {
        // convert privateKey to KeyBlob
        char rawKeyBlob[TEGO_ED25519_KEYBLOB_SIZE] = {0};
        tego_ed25519_keyblob_from_ed25519_private_key(
            rawKeyBlob,
            sizeof(rawKeyBlob),
            privateKey,
            tego::throw_on_error());

        QString keyBlob(rawKeyBlob);

        push_task([=]() -> void
        {
            SettingsObject so(QStringLiteral("identity"));
            so.write("serviceKey", keyBlob);
        });
    }
}

void init_libtego_callbacks(tego_context_t* context)
{
    taskTimer = std::make_unique<QTimer>();
    // fire every 10 ms
    taskTimer->setInterval(10);
    taskTimer->setSingleShot(false);
    QObject::connect(taskTimer.get(), &QTimer::timeout, &consume_tasks);

    taskTimer->start();

    //
    // register each of our callbacks with libtego
    //

    tego_context_set_new_identity_created_callback(
        context,
        &on_new_identity_created,
        tego::throw_on_error());

}