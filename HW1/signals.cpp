#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
    cout << "smash: got ctrl-Z" << endl;
    SmallShell &smash = SmallShell::getInstance();
    if (smash.getCurrentCommand() != nullptr) {
        pid_t pid_signaled = smash.getCurrentCommand()->getMyPid();
        JobsList::JobEntry* temp_job = smash.getJobs().getJobByPID(pid_signaled);
        if (temp_job != nullptr) {
            temp_job->setStop();
        }
        else{
            smash.getJobs().addJob(smash.getCurrentCommand(), true);
        }
        kill(pid_signaled, SIGSTOP);
        cout << "smash: process " << pid_signaled << " was stopped" << endl;
    }
    return;
}

void ctrlCHandler(int sig_num) {
    cout << "smash: got ctrl-C" << endl;
    SmallShell &smash = SmallShell::getInstance();
    if (smash.getCurrentCommand() != nullptr) {
        pid_t pid_signaled = smash.getCurrentCommand()->getMyPid();
        JobsList::JobEntry* temp_job = smash.getJobs().getJobByPID(pid_signaled);
        if (temp_job != nullptr) {
            smash.getJobs().removeJobById(temp_job->getJobID());
        }
        kill(pid_signaled, SIGKILL);
        cout << "smash: process " << pid_signaled << " was killed" << endl;
    }
    return;
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

