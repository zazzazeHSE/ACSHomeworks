#include <iostream>
#include <map>
#include <shared_mutex>
#include <string>
#include <thread>
#include <fstream>
#include <sstream>
#include <vector>
#include <pthread.h>
using namespace std;

std::map<std::string,int> students{{"Egor",2000},{"Artem",1999},{"Maxim",2001}};
std::mutex readerFileMutex;
std::mutex writerFileMutex;
auto start = std::chrono::steady_clock::now();
std::string writerFile;
std::string readerFile;

class Semaphore{
    pthread_mutex_t reader;
    pthread_mutex_t writer;
    pthread_cond_t readerCondition;
    pthread_cond_t writerCondition;
    int readersCount = 0;
    int writersCount = 0;
    
public:
    Semaphore(){
        pthread_mutex_init(&writer, nullptr);
        pthread_mutex_init(&reader, nullptr);
        pthread_cond_init(&writerCondition, nullptr);
        pthread_cond_init(&writerCondition, nullptr);
    }
    
    void readerWait(){
        pthread_mutex_lock(&reader);
        while(writersCount > 0){
            pthread_cond_wait(&writerCondition, &reader);
        }
        readersCount++;
        pthread_mutex_unlock(&reader);
    }
    
    void writerWait(){
        pthread_mutex_lock(&writer);
        while(readersCount > 0 || writersCount > 0){
            pthread_cond_wait(&writerCondition, &writer);
            pthread_cond_wait(&readerCondition, &reader);
        }
        writersCount++;
        pthread_mutex_unlock(&writer);
    }
    
    void writerSignal(){
        pthread_mutex_lock(&writer);
        writersCount--;
        pthread_mutex_unlock(&writer);
        pthread_cond_signal(&readerCondition);
        pthread_cond_signal(&writerCondition);
    }
    
    void readerSignal(){
        pthread_mutex_lock(&reader);
        readersCount--;
        pthread_mutex_unlock(&reader);
        pthread_cond_signal(&readerCondition);
        pthread_cond_signal(&writerCondition);
    }
    
    ~Semaphore(){
        pthread_mutex_destroy(&writer);
        pthread_mutex_destroy(&reader);
        pthread_cond_destroy(&writerCondition);
        pthread_cond_destroy(&readerCondition);
    }
};

Semaphore sem;

void writerWriteInFile(const std::string& msg){
    writerFileMutex.lock();
    std::ofstream file(writerFile, std::ios::app);
    auto end = std::chrono::steady_clock::now();
    file << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0 << ":" << msg << "\n";
    file.close();
    writerFileMutex.unlock();
}

void readerWriteInFile(const std::string& msg){
    readerFileMutex.lock();
    std::ofstream file(readerFile, std::ios::app);
    auto end = std::chrono::steady_clock::now();
    file << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0 << ":" << msg << "\n";
    file.close();
    readerFileMutex.unlock();
}

void addToStudentsList(const std::string& na, int tele){
    sem.writerWait();
    writerWriteInFile("STARTING UPDATE INFO ABOUT STUDENT " + na);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    students[na]= tele;
    writerWriteInFile("ENDING UPDATE INFO ABOUT STUDENT " + na);
    sem.writerSignal();
}

void printInfo(){
    sem.readerWait();
    auto it = students.begin();
    std::advance(it, rand() % students.size());
    std::string txt = it->first + "-";
    readerWriteInFile(txt + std::to_string(students[it->first]));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    sem.readerSignal();
}

int main(int argc, char* argv[]){
    if (argc != 5){
        std::cout << "You should write 4 command line arguments: firtst - count of readers, second - count of writers, third - file for writer info and fourth - file for reader info" << endl;
        return 1;
    }
    ofstream reader(readerFile);
    reader.clear();
    reader.close();
    ofstream writer(writerFile);
    writer.clear();
    writer.close();
    std::stringstream argReaders(argv[1]);
    int readersCount;
    argReaders >> readersCount;
    int writersCount;
    stringstream argWriters(argv[2]);
    argWriters >> writersCount;
    writerFile = argv[3];
    readerFile = argv[4];
    std::vector<std::thread> threads;
    for (int i = 0; i < 2; ++i) {
        std::string name = "";
        for (int j = 0; j < rand() % 10 + 1; ++j) {
            name += rand() % ('a' - 'z') + 'a';
        }
        int year = rand() % 30 + 1970;
        threads.push_back(thread(addToStudentsList, name, year));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    int readersThreadsCount = 0;
    int writersThreadsCount = 0;
    for (int i = 0; i < readersCount + writersCount; ++i) {
        if(rand() % 2 == 0 && readersThreadsCount < readersCount){
            threads.push_back(std::thread([]{printInfo();}));
            readersThreadsCount++;
        }
        else if(writersThreadsCount < writersCount){
            writersThreadsCount++;
            threads.push_back(std::thread([]{
                std::string name = "";
                for (int j = 0; j < rand() % 10 + 1; ++j) {
                    name += rand() % ('a' - 'z') + 'a';
                }
                int year = rand() % 30 + 1970;
                addToStudentsList(name, year);
            }));
        }
        else{
            threads.push_back(std::thread([]{printInfo();}));
            readersThreadsCount++;
        }
    }
    for (int i = 0; i < threads.size(); ++i) {
        threads[i].join();
    }

    std::cout << "\nList of students" << std::endl;
    for (auto student: students){
        std::cout << student.first << ": " << student.second << std::endl;
    }

    std::cout << std::endl;

}
