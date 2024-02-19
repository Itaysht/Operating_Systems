#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
    pid_t current_pid = getpid();
    SmallShell &smash = SmallShell::getInstance();
    if (smash.getCurrentCommand() != nullptr) {
        smash.getJobs().addJob(smash.getCurrentCommand(), true);
        smash.setCurrentCommand(nullptr);
    }
}

void ctrlCHandler(int sig_num) {
    SmallShell &smash = SmallShell::getInstance();
    smash.setCurrentCommand(nullptr);
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

