#include "Commands.h"

using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string _ltrim(const std::string& s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for(std::string s; iss >> s; ) {
    args[i] = (char*)malloc(s.length()+1);
    memset(args[i], 0, s.length()+1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundCommand(const char* cmd_line) {
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

string _removeBackgroundSign(const char* cmd_line) {
  string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos) {
    return str;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&') {
    return str;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  str[idx] = ' ';
  // truncate the command line string up to the last non-space character
  str[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
  return str;
}

// TODO: Add your implementation for classes in Commands.h 

SmallShell::SmallShell() : m_name_of_smash("smash"), m_directory_history(std::stack<std::string>()), m_jobs(new JobsList()), m_current_command(
        nullptr) {
}

std::string SmallShell::getName() { return m_name_of_smash;}

void SmallShell::setName(std::string name_changed) { m_name_of_smash = name_changed;}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (m_cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {
    bool isBack = _isBackgroundCommand(cmd_line);
    string cmd_s = _removeBackgroundSign(cmd_line);
    cmd_s = _trim(cmd_s);
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

    if (firstWord.compare("pwd") == 0) {
        return new GetCurrDirCommand(cmd_line);
    } else if (firstWord.compare("showpid") == 0) {
        return new ShowPidCommand(cmd_line);
    } else if (firstWord.compare("chprompt") == 0) {
        return new ChPromptCommand(cmd_line);
    } else if (firstWord.compare("cd") == 0) {
        return new ChangeDirCommand(cmd_line);
    } else if (firstWord.compare("jobs") == 0) {
        return new JobsCommand(cmd_line);
    } else if (firstWord.compare("quit") == 0) {
        return new QuitCommand(cmd_line);
    } else if (firstWord.compare("kill") == 0) {
        return new KillCommand(cmd_line);
    } else if (firstWord.compare("fg") == 0) {
        return new ForegroundCommand(cmd_line);
    } else if (firstWord.compare("bg") == 0) {
        return new BackgroundCommand(cmd_line);
    }
    else {
        return new ExternalCommand(cmd_s.c_str(), isBack);
    }
    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
  // TODO: Add your implementation here
  // for example:
   Command* cmd = CreateCommand(cmd_line);
   m_current_command = cmd;
   cmd->execute();
   m_current_command = nullptr;
//   Please note that you must fork smash process for some commands (e.g., external commands....)
}

std::stack<std::string>& SmallShell::getStack() {
    return m_directory_history;
}
JobsList& SmallShell::getJobs() {
    return *m_jobs;
}
Command* SmallShell::getCurrentCommand() {
    return m_current_command;
}
void SmallShell::setCurrentCommand(Command *cmd) {
    m_current_command = cmd;
}

Command::Command(const char *cmd_line) : m_cmd_line(std::string(cmd_line)){
    std::string trimmed_cmd_line = cmd_line;
    int num_of_arguments = 0;
    std::vector<std::string> args;
    while (trimmed_cmd_line.find_first_of(" \n") != -1) {
        trimmed_cmd_line = _trim(trimmed_cmd_line);
        trimmed_cmd_line = _trim(trimmed_cmd_line.substr(trimmed_cmd_line.find_first_of(" \n"), trimmed_cmd_line.length()));
        args.push_back(trimmed_cmd_line.substr(0, trimmed_cmd_line.find_first_of(" \n")));
        num_of_arguments++;
    }
    m_number_of_arguments = num_of_arguments;
    m_args = args;
    m_valid_arguments = m_number_of_arguments;
}

std::string Command::commandName() {
    string cmd_s = _trim(m_cmd_line);
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
    for (int i = 0; i < m_valid_arguments; i++){
        firstWord += m_args[i];
    }
    return firstWord;
}

Command::~Command() {}

BuiltInCommand::BuiltInCommand(const char *cmd_line) : Command(cmd_line) {}

ChPromptCommand::ChPromptCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {
    if (m_number_of_arguments == 0){
        new_name = "smash";
        m_valid_arguments = 0;
    }
    else{
        new_name = m_args[0];
        m_valid_arguments = 1;
    }
}

void ChPromptCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    smash.setName(new_name);
}

ShowPidCommand::ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line), m_pid(-1) {
    m_valid_arguments = 0;
}

void ShowPidCommand::execute() {
    if (m_pid == -1){
        m_pid = getpid();
    }
    cout << "smash pid is " << m_pid << endl;
}

GetCurrDirCommand::GetCurrDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {
    m_valid_arguments = 0;
}

void GetCurrDirCommand::execute() {
    char buf[PWD_MAX_LENGTH];
    if (getcwd(buf, sizeof(buf)) == nullptr){
        perror("smash error: getcwd failed");
    }
    cout << string(buf) << endl;
}

ChangeDirCommand::ChangeDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {
    m_valid_arguments = 1;
}

void ChangeDirCommand::execute() {
    if (m_number_of_arguments > 1){
        cerr << "smash error: cd: too many arguments" << endl;
        return;
    }
    SmallShell& smash = SmallShell::getInstance();
    if (m_args[0] == "-") {
        if (smash.getStack().empty()){
            cerr << "smash error: cd: OLDPWD not set" << endl;
            return;
        }
        if (chdir(smash.getStack().top().c_str()) == -1) {
            perror("smash error: chdir failed");
            return;
        }
        smash.getStack().pop();
        return;
    }
    char buf[PWD_MAX_LENGTH];
    if (getcwd(buf, sizeof(buf)) == nullptr){
        perror("smash error: getcwd failed");
        return;
    }
    if (chdir(m_args[0].c_str()) == -1){
        perror("smash error: chdir failed");
        return;
    }
    smash.getStack().push(string(buf));
}


JobsList::JobEntry::JobEntry(int job_id, Command *cmd, bool is_stopped) {
    m_job_id = job_id;
    m_command = cmd;
    m_is_stopped = is_stopped;
    m_pid_of_job = getpid();
    time(&m_start_time);
}

int JobsList::JobEntry::getJobID() {
    return m_job_id;
}

std::string JobsList::JobEntry::getCommandName() {
    return m_command->commandName();
}

pid_t JobsList::JobEntry::getPidOfJob() {
    return m_pid_of_job;
}

bool JobsList::JobEntry::isStopped() {
    return m_is_stopped;
}

time_t JobsList::JobEntry::getStartTime() {
    return m_start_time;
}

void JobsList::JobEntry::setActive() {
    m_is_stopped = false;
}

JobsList::JobEntry::~JobEntry() {}

JobsList::JobsList() {
    m_last_job = 0;
    m_last_stopped_job = 0;
    m_length = 0;
    m_jobs_list = std::vector<JobEntry*>();
    m_jobs_stopped_list = std::vector<JobEntry*>();
}

void JobsList::addJob(Command *cmd, bool isStopped) {
    m_last_job++;
    JobsList::JobEntry* jb = new JobEntry(m_last_job, cmd, isStopped);
    m_jobs_list.push_back(jb);
    if (isStopped){
        m_jobs_stopped_list.push_back(jb);
        m_last_stopped_job++;
    }
    m_length++;
}

JobsList::JobEntry *JobsList::getJobById(int jobId) {
    int i = 0;
    while (i < m_length){
        if (m_jobs_list[i]->getJobID() == jobId){
            return m_jobs_list[i];
        }
        i++;
    }
    return nullptr;
}

void JobsList::removeJobById(int jobId) {
    vector<JobEntry*>::iterator v, u;
    for (v = m_jobs_list.begin(); v != m_jobs_list.end(); v++){
        if ((*v)->getJobID() == jobId){
            if (jobId == m_last_job){
                m_jobs_list.pop_back();
                m_last_job = m_jobs_list.back()->getJobID();
            }
            else {
                m_jobs_list.erase(v);
            }
            if ((*v)->isStopped()){
                for (u = m_jobs_stopped_list.begin(); u != m_jobs_stopped_list.end(); u++){
                    if ((*u)->getJobID() == jobId) {
                        if (jobId == m_last_stopped_job){
                            m_jobs_stopped_list.pop_back();
                            m_last_stopped_job = m_jobs_stopped_list.back()->getJobID();
                        }
                        else {
                            m_jobs_stopped_list.erase(u);
                        }
                        break;
                    }
                }
            }
            m_length--;
            return;
        }
    }
}

void JobsList::printJobsList() {
    time_t finish;
    cout << m_jobs_list[0]->getJobID() << endl;
    cout << m_jobs_list[0]->getCommandName() << endl;
    cout << m_jobs_list[0]->getPidOfJob() << endl;
    cout << "damn nigga" << endl;
    for (int i = 0; i < m_length; i++){
        cout << "[" << m_jobs_list[i]->getJobID() << "] " << m_jobs_list[i]->getCommandName() <<
        " : " << m_jobs_list[i]->getPidOfJob() << " " << difftime(time(&finish), m_jobs_list[i]->getStartTime()) <<
        " secs";
        if (m_jobs_list[i]->isStopped()){
            cout << " (stopped)" << endl;
        }
        else{
            cout << endl;
        }
    }
}

bool JobsList::isItEmpty() {
    return (m_length == 0);
}

bool JobsList::isStoppedEmpty() {
    return m_jobs_stopped_list.empty();
}

JobsList::~JobsList() {}

void JobsList::killAllJobs() {
    cout << "smash: sending SIGKILL signal to " << m_length << " jobs:" << endl;
    for (int i = 0; i < m_length; i++){
        cout << m_jobs_list[i]->getPidOfJob() << ": " << m_jobs_list[i]->getCommandName() << endl;
    }
    m_jobs_stopped_list.clear();
    m_jobs_list.clear();
}

JobsCommand::JobsCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {
    m_valid_arguments = 0;
}

void JobsCommand::execute() {
    SmallShell& smash = SmallShell::getInstance();
    smash.getJobs().printJobsList();
}

ForegroundCommand::ForegroundCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {
    if (m_number_of_arguments > 1){
        m_valid_arguments = 1;
    }
}

bool isTheStringInt(string num){
    int length = num.size();
    for (int i = 0; i < length; i++){
        if (!(isdigit(num[i]))){
            return false;
        }
    }
    return true;
}

void ForegroundCommand::execute() {
    if ((m_number_of_arguments > 1) || (!(isTheStringInt(m_args[0])))){
        cerr << "smash error: fg: invalid arguments" << endl;
        return;
    }
    SmallShell& smash = SmallShell::getInstance();
    if ((m_number_of_arguments == 0) && (smash.getJobs().isItEmpty())){
        cerr << "smash error: fg: jobs list is empty" << endl;
        return;
    }
    int temp_job_id = smash.getJobs().m_last_job;
    if (m_number_of_arguments > 0){
        temp_job_id = stoi(m_args[0]);
    }
    JobsList::JobEntry* jb = smash.getJobs().getJobById(temp_job_id);
    if (jb == nullptr){
        cerr << "smash error: fg: job-id " << temp_job_id << " does not exist" << endl;
        return;
    }
    cout << commandName() << " : " << jb->getPidOfJob() << endl;
    smash.getJobs().removeJobById(temp_job_id);
    kill(jb->getPidOfJob(), SIGCONT);
    waitpid(jb->getPidOfJob());
}

BackgroundCommand::BackgroundCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {
    if (m_number_of_arguments > 1){
        m_valid_arguments = 1;
    }
}

void BackgroundCommand::execute() {
    if ((m_number_of_arguments > 1) || (!(isTheStringInt(m_args[0])))){
        cerr << "smash error: bg: invalid arguments" << endl;
        return;
    }
    SmallShell& smash = SmallShell::getInstance();
    if ((m_number_of_arguments == 0) && (smash.getJobs().isStoppedEmpty())){
        cerr << "smash error: bg: there is no stopped jobs to resume" << endl;
        return;
    }
    int temp_job_id = stoi(m_args[0]);
    JobsList::JobEntry* jb = smash.getJobs().getJobById(temp_job_id);
    if (jb == nullptr){
        cerr << "smash error: bg: job-id " << temp_job_id << " does not exist" << endl;
        return;
    }
    if (!(jb->isStopped())){
        cerr << "smash error: bg: job-id " << temp_job_id << " is already running in the background" << endl;
        return;
    }
    vector<JobsList::JobEntry*>::iterator v;
    for (v = smash.getJobs().m_jobs_stopped_list.begin(); v != smash.getJobs().m_jobs_stopped_list.end(); v++){
        if ((*v)->getJobID() == temp_job_id){
            if (temp_job_id == smash.getJobs().m_last_stopped_job){
                smash.getJobs().m_jobs_stopped_list.pop_back();
                smash.getJobs().m_last_stopped_job = smash.getJobs().m_jobs_stopped_list.back()->getJobID();
            }
            else {
                smash.getJobs().m_jobs_stopped_list.erase(v);
            }
            (*v)->setActive();
            cout << (*v)->getCommandName() << " : " << (*v)->getPidOfJob() << endl;
            return;
        }
    }
}

QuitCommand::QuitCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

void QuitCommand::execute() {
    if (m_valid_arguments == 0){
        exit(0);
    }
    if (m_args[0] == "kill"){
        SmallShell& smash = SmallShell::getInstance();
        smash.getJobs().killAllJobs();
    }
    exit(0);
}

KillCommand::KillCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {
    m_valid_arguments = 2;
}

void KillCommand::execute() {
    string num_of_signal = m_args[0].substr(1, (m_args[0].size()));
    if ((m_number_of_arguments != 2) || (m_args[0][0] != '-') ||
            (!(isTheStringInt(num_of_signal))) || (!(isTheStringInt(m_args[1])))
            || (stoi(num_of_signal) > 31)){
        cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }
    SmallShell& smash = SmallShell::getInstance();
    JobsList::JobEntry* jb = smash.getJobs().getJobById(stoi(m_args[1]));
    if (jb == nullptr){
        cerr << "smash error: kill: job-id " << stoi(m_args[1]) << " does not exist" << endl;
        return;
    }

    if (kill(jb->getPidOfJob(), stoi(num_of_signal)) == -1){
        perror("smash error: kill failed");
        return;
    }
    cout << "signal number " << stoi(num_of_signal) << " was sent to pid " << jb->getPidOfJob() << endl;
}

ExternalCommand::ExternalCommand(const char *cmd_line, bool isBack) : Command(cmd_line), m_is_back(isBack) {}

void ExternalCommand::execute() {
    SmallShell &smash = SmallShell::getInstance();
    char **arguments = new char *[25];
    _parseCommandLine(m_cmd_line.c_str(), arguments);

    const char *fileName = (("/bin/") + string(arguments[0])).c_str();
    pid_t process_id = fork();
    if (process_id == -1) {
        perror("smash error: fork failed");
        return;
    }
    if (process_id == 0) {
        if (m_is_back) {
            smash.getJobs().addJob(this);
        }
        execv(fileName, arguments);
    }
    else {
        if (!m_is_back) {
            wait(NULL);
        }
    }
}