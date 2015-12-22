#include <systemlib/err.h>
#include <unistd.h>
#include <errno.h>

extern "C" __EXPORT int insider_main(int argc, char *argv[]);

class Insider
{
public:
    int    start();
    Insider();
    ~Insider();
private:
    int		_control_task;			/**< task handle */
    bool	_task_should_exit;		/**< if true, task() should exit */
    int     task();
    static int insider_task(int argc, char *argv[]);
};

Insider * insiderPtr;

Insider::Insider(): _control_task(-1), _task_should_exit(false)
{


}


Insider::~Insider()
{
    _task_should_exit = true;
    for (unsigned int i = 0; _control_task != -1; i++) {

        usleep(20000);

        /* if we have given up, kill it */
        if (i > 50) {
            px4_task_delete(_control_task);
            break;
        }
    }
}



int Insider::insider_task(int argc, char *argv[])
{
    return insiderPtr->task();
}

int Insider::task()
{
    while (true) {
        warnx("Hello from insider!\n");
        usleep(10000000);
        if (_task_should_exit) break;
    }

    return 0;
}

int
Insider::start()
{
    ASSERT(_control_task == -1);

    /* start the task */
    _control_task = px4_task_spawn_cmd("insider",
                       SCHED_DEFAULT,
                       SCHED_PRIORITY_MAX - 5,
                       1500,
                       (px4_main_t)&Insider::insider_task,
                       nullptr);

    if (_control_task < 0) {
        warn("insider start failed");
        return -errno;
    }

    return OK;
}

int insider_main(int argc, char *argv[])
{

    if (argc < 2) {
        warnx("usage: insider {start|stop|status}");
        return 1;
    }

    if (!strcmp(argv[1], "start")) {

        if (insiderPtr != nullptr) {
            warnx("already running");
            return 1;
        }

        insiderPtr = new Insider;

        if (insiderPtr == nullptr) {
            warnx("alloc failed");
            return 1;
        }

        if (OK != insiderPtr->start()) {
            delete insiderPtr;
            insiderPtr = nullptr;
            warnx("start failed");
            return 1;
        }

        return 0;
    }

    if (!strcmp(argv[1], "stop")) {
        if (insiderPtr == nullptr) {
            warnx("not running");
            return 1;
        }

        delete insiderPtr;
        insiderPtr = nullptr;
        return 0;
    }

    if (!strcmp(argv[1], "status")) {
        warnx(insiderPtr ? "running" : "not running");
        return 0;
    }

    warnx("unrecognized command");
    return 1;
}

