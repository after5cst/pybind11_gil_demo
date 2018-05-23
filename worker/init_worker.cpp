#include "init_worker.h"
#include "launch.h"
#include "job.h"

void worker::init_worker(pybind11::module &module)
{
    worker::bind_worker_input(module);
    worker::bind_worker_job(module);
    worker::bind_worker_launch(module);
    worker::bind_worker_state(module);
}
