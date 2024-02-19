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

bool _isComplexCommand(std::string cmd){
    int length = cmd.length();
    for (int i=0; i<length; i++){
        if ((cmd[i] == '*') || (cmd[i] == '?')){
            return true;
        }
    }
    return false;
}

bool _isRedirectOrPipe(std::string cmd, char symbol){
    int length = cmd.length();
    for (int i=0; i<length; i++){
        if (cmd[i] == symbol){
            return true;
        }
    }
    return false;
}
bool _isRedirectAndAppendOrPiped(std::string cmd, char symbol1, char symbol2){
    int length = cmd.length();
    for (int i=0; i<length-1; i++){
        if ((cmd[i] == symbol1) && (cmd[i+1] == symbol2))
        {
            return true;
        }
    }
    return false;
}
bool _isFileExist(const char* cmd_line){
    int fd = open(cmd_line, O_RDONLY);
    if (fd == -1){
        return false;
    }
    close(fd);
    return true;
}


string _removeBackgroundSign(const char* cmd_line) {
  string str(cmd_line);
  // find last character other than spaces
  int idx = str.find_last_not_of(WHITESPACE);
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

SmallShell::SmallShell() : m_name_of_smash("smash"), m_last_pw(""), m_jobs(new JobsList()), m_current_command(
        nullptr) {
}

std::string SmallShell::getName() { return m_name_of_smash;}

std::string SmallShell::getLastPw() { return m_last_pw;}

void SmallShell::setLastPw(std::string pw) { m_last_pw = pw;}

void SmallShell::setName(std::string name_changed) { m_name_of_smash = name_changed;}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (m_cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {
    bool isBack = _isBackgroundCommand(cmd_line);
    bool is_redirect = _isRedirectOrPipe(string(cmd_line), '>');
    bool is_piped = _isRedirectOrPipe(string(cmd_line), '|');
    string cmd_s = _removeBackgroundSign(cmd_line);
    cmd_s = _trim(cmd_s);
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

    if (is_redirect){
        if (_isRedirectAndAppendOrPiped(string(cmd_line), '>', '>')){
            return new RedirectionCommand(cmd_line, true);
        }
        return new RedirectionCommand(cmd_line);
    }
    if (is_piped){
        if (_isRedirectAndAppendOrPiped(string(cmd_line), '|', '&')) {
            return new PipeCommand(cmd_line, true);
        }
        return new PipeCommand(cmd_line);
    }

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
    else if (firstWord.compare("setcore") == 0){
        return new SetcoreCommand(cmd_line);
    }
    else if (firstWord.compare("getfiletype") == 0){
        return new GetFileTypeCommand(cmd_line);
    }
    else if (firstWord.compare("chmod") == 0){
        return new ChmodCommand(cmd_line);
    }
    else {
        return new ExternalCommand(cmd_line, isBack);
    }
    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    // for example:
    Command *cmd = CreateCommand(cmd_line);
    setCurrentCommand(cmd);
    cmd->execute();
    setCurrentCommand(nullptr);
//   Please note that you must fork smash process for some commands (e.g., external commands....)
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
    trimmed_cmd_line = _trim(trimmed_cmd_line);
    while (trimmed_cmd_line.find_first_of(" \n") != -1) {
        trimmed_cmd_line = _trim(trimmed_cmd_line.substr(trimmed_cmd_line.find_first_of(" \n"), trimmed_cmd_line.length()));
        args.push_back(trimmed_cmd_line.substr(0, trimmed_cmd_line.find_first_of(" \n")));
        num_of_arguments++;
        trimmed_cmd_line = _trim(trimmed_cmd_line);
    }
    m_number_of_arguments = num_of_arguments;
    m_args = args;
    m_valid_arguments = m_number_of_arguments;
    m_my_pid = getpid();
    m_name = m_cmd_line;
//    string cmd_s = _trim(m_cmd_line);
//    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n")) + " ";
//    for (int i = 0; i < m_valid_arguments; i++){
//        firstWord += m_args[i];
//    }
//    m_name = firstWord;
}

pid_t Command::getMyPid() {
    return m_my_pid;
}

void Command::setMyPid(pid_t new_pid) {
    m_my_pid = new_pid;
}

std::string Command::commandName() {
    return m_name;
}

void Command::setCommandName(std::string name) {
    m_name = name;
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

ShowPidCommand::ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {
    m_valid_arguments = 0;
}

void ShowPidCommand::execute() {
    cout << "smash pid is " << m_my_pid << endl;
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
    char buf[PWD_MAX_LENGTH];
    if (getcwd(buf, sizeof(buf)) == nullptr){
        perror("smash error: getcwd failed");
        return;
    }
    SmallShell& smash = SmallShell::getInstance();
    if (m_args[0] == "-") {
        if (smash.getLastPw() == ""){
            cerr << "smash error: cd: OLDPWD not set" << endl;
            return;
        }
        if (chdir(smash.getLastPw().c_str()) == -1) {
            perror("smash error: chdir failed");
            return;
        }
    }
    else{
        if (chdir(m_args[0].c_str()) == -1){
            perror("smash error: chdir failed");
            return;
        }
    }
    smash.setLastPw(string(buf));
}


JobsList::JobEntry::JobEntry(int job_id, Command *cmd, bool is_stopped) {
    m_job_id = job_id;
    m_command = cmd;
    m_is_stopped = is_stopped;
    time(&m_start_time);
}

int JobsList::JobEntry::getJobID() {
    return m_job_id;
}

Command *JobsList::JobEntry::getCommand() {
    return m_command;
}

std::string JobsList::JobEntry::getCommandName() {
    return m_command->commandName();
}

pid_t JobsList::JobEntry::getPidOfJob() {
    return m_command->getMyPid();
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
void JobsList::JobEntry::setStop() {
    m_is_stopped = true;
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
    removeFinishedJobs();
    m_last_job++;
    JobsList::JobEntry* jb = new JobEntry(m_last_job, cmd, isStopped);
    m_jobs_list.push_back(jb);
    if (isStopped){
        m_jobs_stopped_list.push_back(jb);
        m_last_stopped_job = m_last_job;
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

JobsList::JobEntry *JobsList::getJobByPID(pid_t pid_check) {
    int i = 0;
    while (i < m_length){
        if (m_jobs_list[i]->getPidOfJob() == pid_check){
            return m_jobs_list[i];
        }
        i++;
    }
    return nullptr;
}

void JobsList::removeJobById(int jobId) {
    vector<JobEntry *>::iterator v, u;
    bool stopped = true;
    for (v = m_jobs_list.begin(); v != m_jobs_list.end(); v++) {
        if ((*v)->getJobID() == jobId) {
            if (!((*v)->isStopped())) {
                stopped = false;
            }
            if (jobId == m_last_job) {
                m_jobs_list.pop_back();
                if (m_jobs_list.empty()) {
                    m_last_job = 0;
                } else {
                    m_last_job = m_jobs_list.back()->getJobID();
                }
            } else {
                m_jobs_list.erase(v);
            }
            break;
        }
    }
    if (stopped) {
        for (u = m_jobs_stopped_list.begin(); u != m_jobs_stopped_list.end(); u++) {
            if ((*u)->getJobID() == jobId) {
                if (jobId == m_last_stopped_job) {
                    m_jobs_stopped_list.pop_back();
                    if (m_jobs_stopped_list.empty()) {
                        m_last_stopped_job = 0;
                    } else {
                        m_last_stopped_job = m_jobs_stopped_list.back()->getJobID();
                    }
                } else {
                    m_jobs_stopped_list.erase(u);
                }
                break;
            }
        }
    }
    m_length--;
}

void JobsList::printJobsList() {
    time_t finish;
    removeFinishedJobs();
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
    removeFinishedJobs();
    cout << "smash: sending SIGKILL signal to " << m_length << " jobs:" << endl;
    for (int i = 0; i < m_length; i++){
        cout << m_jobs_list[i]->getPidOfJob() << ": " << m_jobs_list[i]->getCommandName() << endl;
        kill(m_jobs_list[i]->getPidOfJob(), SIGKILL);
    }
    m_jobs_stopped_list.clear();
    m_jobs_list.clear();
}

void JobsList::removeFinishedJobs() {
    int status = -1;
    for (int i = 0; i < m_length; i++) {
        waitpid(m_jobs_list[i]->getPidOfJob(), &status, WNOHANG);
        if (WIFEXITED(status)) {
            removeJobById(m_jobs_list[i]->getJobID());
            i--;
        }
        else{
            if (WIFSIGNALED(status)){            // need to find another way to catch from kill command
                int signum = WTERMSIG(status);
                if ((signum == 9) || (signum == 2)){           //SIGKILL, SIGINT
                    removeJobById(m_jobs_list[i]->getJobID());
                    i--;
                }
            }
        }
        status = 1;
    }
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
    if ((num[0] != '-') && (!isdigit(num[0]))){
        return false;
    }
    for (int i = 1; i < length; i++){
        if (!(isdigit(num[i]))){
            return false;
        }
    }
    return true;
}
int octalToDecimal(int n)
{
    int num = n;
    int dec_value = 0;
    int base = 1;
    int temp = num;
    while (temp) {
        int last_digit = temp % 10;
        temp = temp / 10;
        dec_value += last_digit * base;
        base = base * 8;
    }
    return dec_value;
}

void ForegroundCommand::execute() {
    if (m_number_of_arguments > 1){
        cerr << "smash error: fg: invalid arguments" << endl;
        return;
    }
    SmallShell& smash = SmallShell::getInstance();
    smash.getJobs().removeFinishedJobs();
    if ((m_number_of_arguments == 0) && (smash.getJobs().isItEmpty())){
        cerr << "smash error: fg: jobs list is empty" << endl;
        return;
    }
    int temp_job_id = smash.getJobs().m_last_job;
    if ((m_number_of_arguments > 0) && (isTheStringInt(m_args[0]))){
        temp_job_id = stoi(m_args[0]);
    }
    else{
        if (m_number_of_arguments != 0){
            cerr << "smash error: fg: invalid arguments" << endl;
            return;
        }
    }
    JobsList::JobEntry* jb = smash.getJobs().getJobById(temp_job_id);
    if (jb == nullptr){
        cerr << "smash error: fg: job-id " << temp_job_id << " does not exist" << endl;
        return;
    }
    cout << jb->getCommandName() << " : " << jb->getPidOfJob() << endl;
    smash.setCurrentCommand(jb->getCommand());
    kill(jb->getPidOfJob(), SIGCONT);
    int status;
    waitpid(jb->getPidOfJob(), &status, WUNTRACED);
    if (WIFEXITED(status)) {
        smash.getJobs().removeJobById(temp_job_id);
    }
}

BackgroundCommand::BackgroundCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {
    if (m_number_of_arguments > 1){
        m_valid_arguments = 1;
    }
}

void BackgroundCommand::execute() {
    if (m_number_of_arguments > 1){
        cerr << "smash error: bg: invalid arguments" << endl;
        return;
    }
    SmallShell& smash = SmallShell::getInstance();
    if ((m_number_of_arguments == 0) && (smash.getJobs().isStoppedEmpty())){
        cerr << "smash error: bg: there is no stopped jobs to resume" << endl;
        return;
    }
    int temp_job_id = smash.getJobs().m_last_stopped_job;
    if (m_number_of_arguments > 0) {
        if (isTheStringInt(m_args[0])) {
            temp_job_id = stoi(m_args[0]);
        }
        else {
            cerr << "smash error: bg: invalid arguments" << endl;
            return;
        }
    }
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
            (*v)->setActive();
            cout << (*v)->getCommandName() << " : " << (*v)->getPidOfJob() << endl;
            if (temp_job_id == smash.getJobs().m_last_stopped_job){
                smash.getJobs().m_jobs_stopped_list.pop_back();
                if (smash.getJobs().m_jobs_stopped_list.empty()){
                    smash.getJobs().m_last_stopped_job = 0;
                }
                else {
                    smash.getJobs().m_last_stopped_job = smash.getJobs().m_jobs_stopped_list.back()->getJobID();
                }
            }
            else {
                smash.getJobs().m_jobs_stopped_list.erase(v);
            }
            kill(temp_job_id, SIGCONT);
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
    SmallShell& smash = SmallShell::getInstance();
    smash.getJobs().removeFinishedJobs();
    if ((m_number_of_arguments < 2) || (!(isTheStringInt(m_args[1])))){
        cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }
    JobsList::JobEntry* jb = smash.getJobs().getJobById(stoi(m_args[1]));
    if (jb == nullptr){
        cerr << "smash error: kill: job-id " << stoi(m_args[1]) << " does not exist" << endl;
        return;
    }
    if ((m_number_of_arguments != 2) || (m_args[0][0] != '-') ||
        (!(isTheStringInt(m_args[0].substr(1, (m_args[0].size())))))){
        cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }
    string num_of_signal = m_args[0].substr(1, (m_args[0].size()));

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
    bool is_complex = _isComplexCommand(m_cmd_line);
    if (m_is_back){
        _parseCommandLine(_removeBackgroundSign(m_cmd_line.c_str()).c_str(), arguments);
    }
    else {
        _parseCommandLine(m_cmd_line.c_str(), arguments);
    }
    pid_t process_id = fork();
    if (process_id == -1) {
        perror("smash error: fork failed");
        return;
    }
    if (process_id == 0) {
        setpgrp();
        if (is_complex) {
            char **comp_args = new char *[4];
            comp_args[0] = (char *) malloc(10);
            memset(comp_args[0], 0, 10);
            strcpy(comp_args[0], "/bin/bash");
            comp_args[1] = (char *) malloc(2);
            memset(comp_args[1], 0, 2);
            strcpy(comp_args[1], "-c");
            comp_args[2] = (char *) malloc(m_cmd_line.length() + 1);
            memset(comp_args[2], 0, m_cmd_line.length() + 1);
            strcpy(comp_args[2], m_cmd_line.c_str());
            comp_args[3] = NULL;
            if (execv("/bin/bash", comp_args) == -1) {
                perror("smash error: execv failed");
                exit(1);
            }
        } else {
            if (execvp(arguments[0], arguments) == -1) {
                perror("smash error: execvp faild");
                exit(1);
            }
        }
    }
    else {
        if (m_is_back) {
            this->setMyPid(process_id);
            smash.getJobs().addJob(this);
        }
        else {
            this->setMyPid(process_id);
            waitpid(process_id, nullptr, WUNTRACED);
            smash.setCurrentCommand(nullptr);
        }
    }
}


RedirectionCommand::RedirectionCommand(const char *cmd_line, bool append) : Command(cmd_line){
    string cmd_s = string(cmd_line);
    m_append = append;
    size_t first = cmd_s.find_first_of(">");
    string lefty = cmd_s.substr(0, first);
    if (m_append){
        first++;
    }
    m_right_output = _trim(cmd_s.substr(first+1, cmd_s.length()));
    m_exist = _isFileExist(m_right_output.c_str());
    SmallShell& smash = SmallShell::getInstance();
    m_left_command = smash.CreateCommand(lefty.c_str());
}

void RedirectionCommand::execute() {
    int original_output = dup(STDOUT_FILENO);
    pid_t father_pid = getMyPid();
    pid_t curr_pid = fork();
    if (curr_pid == 0) {
        int output_fd = 0;
        if (m_append && m_exist){
            output_fd = open(m_right_output.c_str(), O_WRONLY | O_APPEND, 0666);
        }
        else {
            output_fd = open(m_right_output.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
        }
        if (output_fd == -1){
            perror("smash error: open failed");
            _exit(1);
        }
        dup2(output_fd, STDOUT_FILENO);
        close(output_fd);
        m_left_command->setMyPid(father_pid);
        m_left_command->execute();
        _exit(0);
    }
    else{
        waitpid(curr_pid, NULL, WUNTRACED);
        dup2(original_output, STDOUT_FILENO);
        close(original_output);
    }
}

PipeCommand::PipeCommand(const char *cmd_line, bool isstderr) : Command(cmd_line) {
    string cmd_s = string(cmd_line);
    size_t first = cmd_s.find_first_of("|");
    string lefty = cmd_s.substr(0, first);
    if (isstderr){
        first++;
    }
    string righty = _trim(cmd_s.substr(first+1, cmd_s.length()));
    SmallShell& smash = SmallShell::getInstance();
    m_left_command = smash.CreateCommand(lefty.c_str());
    m_right_command = smash.CreateCommand(righty.c_str());
    m_isstderr = isstderr;
}

void PipeCommand::execute() {
    int original_output = dup(STDIN_FILENO);
    int piper[2];
    pipe(piper);
    pid_t father_pid = getMyPid();
    pid_t curr_pid = fork();
    if (curr_pid == 0){  //son writes
        close(piper[0]);
        if (m_isstderr) {
            dup2(piper[1], STDERR_FILENO);
        }
        else {
            dup2(piper[1], STDOUT_FILENO);
        }
        m_left_command->setMyPid(father_pid);
        m_left_command->execute();
        _exit(0);
    }
    else{   //father reads
        close(piper[1]);
        dup2(piper[0], STDIN_FILENO);
        m_right_command->execute();
        dup2(original_output, STDIN_FILENO);
    }
}

SetcoreCommand::SetcoreCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {
    m_valid_arguments = 2;
}

void SetcoreCommand::execute() {
    bool invld_args = false;
    int core_number = -1;
    pid_t pid_of_given_job = -1;
    SmallShell& smash = SmallShell::getInstance();
    if (isTheStringInt(m_args[0])){
        if (smash.getJobs().getJobById(stoi(m_args[0])) == nullptr){
            cerr << "smash error: setcore: job-id " << m_args[0] << " does not exist"  << endl;
            return;
        }
        pid_of_given_job = smash.getJobs().getJobById(stoi(m_args[0]))->getPidOfJob();
    }
    else{
        invld_args = true;
    }
    if (isTheStringInt(m_args[1])){
        if ((stoi(m_args[1]) >= get_nprocs()) || (stoi(m_args[1]) < 0)){
            cerr << "smash error: setcore: invalid core number" << endl;
            return;
        }
        core_number = stoi(m_args[1]);
    }
    else{
        invld_args = true;
    }
    if ((invld_args) || (m_number_of_arguments != 2)){
        cerr << "smash error: setcore: invalid arguments" << endl;
        return;
    }
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(core_number, &mask);
    sched_setaffinity(pid_of_given_job, sizeof(mask), &mask);
}

GetFileTypeCommand::GetFileTypeCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {
    m_valid_arguments = 1;
}

void GetFileTypeCommand::execute() {
    struct stat buf;
    if ((m_number_of_arguments != 1) || (lstat(m_args[0].c_str(), &buf) == -1)){
        cerr << "smash error: gettype: invalid arguments" << endl;
        return;
    }
    mode_t mode_of_file = buf.st_mode;
    string ans_of_file_type = "";
    if (S_ISREG(mode_of_file)){
        ans_of_file_type = "regular file";
    }
    if (S_ISDIR(mode_of_file)){
        ans_of_file_type = "directory";
    }
    if (S_ISCHR(mode_of_file)){
        ans_of_file_type = "character device";
    }
    if (S_ISBLK(mode_of_file)){
        ans_of_file_type = "block device";
    }
    if (S_ISFIFO(mode_of_file)){
        ans_of_file_type = "FIFO";
    }
    if (S_ISLNK(mode_of_file)){
        ans_of_file_type = "symbolic link";
    }
    if (S_ISSOCK(mode_of_file)){
        ans_of_file_type = "socket";
    }
    cout << m_args[0] << "'s type is \"" << ans_of_file_type << "\" and takes up " << buf.st_size << " bytes" << endl;
}

ChmodCommand::ChmodCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {
    m_valid_arguments = 2;
}

void ChmodCommand::execute() {
    if ((m_number_of_arguments != 2) || (!(isTheStringInt(m_args[0])))){
        cerr << "smash error: chmod: invalid arguments" << endl;
        return;
    }
    if (chmod(m_args[1].c_str(), octalToDecimal(stoi(m_args[0]))) == -1){
        cerr << "smash error: chmod: invalid arguments" << endl;
        return;
    }
}
