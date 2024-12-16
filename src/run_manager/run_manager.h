//
// Created by dmerkushov on 03.02.23.
//

#ifndef SOAPSTAB_RUN_MANAGER_H
#define SOAPSTAB_RUN_MANAGER_H

#include <atomic>

namespace soapstab {

/**
 * @brief A singleton class designed to set the running-stopping state of the application
 */
class run_manager final {
public:
    static run_manager &instance() {
        static run_manager instance;
        return instance;
    }
    run_manager(const run_manager &) = delete;
    void operator=(const run_manager &) = delete;

    /**
     * @return Is the bot currently in running state?
     */
    bool running();

    /**
     * @brief Tell the bot to stop
     *
     * This will cause ending the while-loop in the <code>main()</code> function, and after that loop the <code>main()</code> function will attempt to stop all the threads in a safe manner
     */
    void stop();

private:
    run_manager() = default;

    std::atomic_bool _running = true;
};

} // namespace soapstab

#endif // SOAPSTAB_RUN_MANAGER_H
