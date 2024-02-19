#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <stack>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include <signal.h>
#include <fcntl.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define PWD_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

class Command {
protected:
    std::string m_cmd_line;
    std::vector<std::string> m_args;
    int m_number_of_arguments;
    int m_valid_arguments;
    pid_t m_my_pid;
    std::string m_name;
public:
  Command(const char* cmd_line);
  virtual ~Command();
  virtual void execute() = 0;
  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed
  std::string commandName();
  void setCommandName(std::string name);
  pid_t getMyPid();
  void setMyPid(pid_t new_pid);
};

class BuiltInCommand : public Command {
 public:
  BuiltInCommand(const char* cmd_line);
  virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
    bool m_is_back;
 public:
  ExternalCommand(const char* cmd_line, bool isBack);
  virtual ~ExternalCommand() {}
  void execute() override;
};

class PipeCommand : public Command {
    Command* m_left_command;
    Command* m_right_command;
    bool m_isstderr;
 public:
  PipeCommand(const char* cmd_line, bool isstderr = false);
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command {
    Command* m_left_command;
    std::string m_right_output;
    bool m_append;
    bool m_exist;
 public:
  explicit RedirectionCommand(const char* cmd_line, bool append = false);
  virtual ~RedirectionCommand() {}
  void execute() override;
  //void prepare() override;
  //void cleanup() override;
};

class ChangeDirCommand : public BuiltInCommand {      //cd
public:
  ChangeDirCommand(const char* cmd_line);
  virtual ~ChangeDirCommand() {}
  void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {    //pwd
 public:
  GetCurrDirCommand(const char* cmd_line);
  virtual ~GetCurrDirCommand() {}
  void execute() override;
};

class ChPromptCommand : public BuiltInCommand {        //chprompt
    std::string new_name;
public:
    ChPromptCommand(const char* cmd_line);
    virtual ~ChPromptCommand() {}
    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {       //showpid
 public:
  ShowPidCommand(const char* cmd_line);
  virtual ~ShowPidCommand() {}
  void execute() override;
};

class JobsList;
class QuitCommand : public BuiltInCommand {             //quit
// TODO: Add your data members
public:
  QuitCommand(const char* cmd_line);        //change signature here
  virtual ~QuitCommand() {}
  void execute() override;
};


class JobsList {
 public:
  class JobEntry {
  private:
      int m_job_id;
      Command* m_command;
      bool m_is_stopped;
      time_t m_start_time;
  public:
      JobEntry(int job_id, Command* cmd, bool is_stopped);
      ~JobEntry();
      int getJobID();
      std::string getCommandName();
      Command* getCommand();
      bool isStopped();
      pid_t getPidOfJob();
      time_t getStartTime();
      void setActive();
      void setStop();
  };

  int m_last_job;
  int m_last_stopped_job;
  int m_length;
  std::vector<JobEntry*> m_jobs_list;
  std::vector<JobEntry*> m_jobs_stopped_list;

 public:
  JobsList();
  ~JobsList();
  void addJob(Command* cmd, bool isStopped = false);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry * getJobById(int jobId);
  JobEntry * getJobByPID(pid_t pid_check);
  void removeJobById(int jobId);
  JobEntry * getLastJob(int* lastJobId);
  JobEntry *getLastStoppedJob(int *jobId);
  // TODO: Add extra methods or modify exisitng ones as needed
  bool isItEmpty();
  bool isStoppedEmpty();
};

class JobsCommand : public BuiltInCommand {                  //jobs
 // TODO: Add your data members
 public:
  JobsCommand(const char* cmd_line);       //change signature here
  virtual ~JobsCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {              //fg
 // TODO: Add your data members
 public:
  ForegroundCommand(const char* cmd_line);         //change signature here
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {             //bg
 // TODO: Add your data members
 public:
  BackgroundCommand(const char* cmd_line);    //change signatrue here
  virtual ~BackgroundCommand() {}
  void execute() override;
};

class TimeoutCommand : public BuiltInCommand {
/* Bonus */
// TODO: Add your data members
 public:
  explicit TimeoutCommand(const char* cmd_line);
  virtual ~TimeoutCommand() {}
  void execute() override;
};

class ChmodCommand : public BuiltInCommand {
  // TODO: Add your data members
 public:
  ChmodCommand(const char* cmd_line);
  virtual ~ChmodCommand() {}
  void execute() override;
};

class GetFileTypeCommand : public BuiltInCommand {
  // TODO: Add your data members
 public:
  GetFileTypeCommand(const char* cmd_line);
  virtual ~GetFileTypeCommand() {}
  void execute() override;
};

class SetcoreCommand : public BuiltInCommand {
  // TODO: Add your data members
 public:
  SetcoreCommand(const char* cmd_line);
  virtual ~SetcoreCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand {           //kill
 // TODO: Add your data members
 public:
  KillCommand(const char* cmd_line);     // change signature here
  virtual ~KillCommand() {}
  void execute() override;
};

class SmallShell {
 private:
    std::string m_name_of_smash;
    std::string m_last_pw;
    JobsList* m_jobs;
    Command* m_current_command;
  SmallShell();
 public:
  Command *CreateCommand(const char* cmd_line);
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
//  static SmallShell& make(std::istream& in, std::ostream& out, std::ostream& err)     //remove later!!
//  {
//      std::cin.rdbuf(in.rdbuf());
//      std::cout.rdbuf(out.rdbuf());
//      std::cerr.rdbuf(err.rdbuf());
//      return getInstance();
//  }
//  void run();
  ~SmallShell();
  std::string getName();
  std::string getLastPw();
  void setLastPw(std::string pw);
  void setName(std::string name_changed);
  void executeCommand(const char* cmd_line);
  JobsList& getJobs();
  Command* getCurrentCommand();
  void setCurrentCommand(Command* cmd);
  // TODO: add extra methods as needed
};

#endif //SMASH_COMMAND_H_
