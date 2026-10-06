#include <iostream>
#include <string>
#include <ctime>
#include <cctype>
#include <iomanip>
using namespace std;

const int MAX_SIZE = 100;

const string SYMPTOMS[] = {
    "Cardiac Arrest", "Not Breathing", "Unconsciousness", "Severe Shock", "Severe Bleeding",
    "Chest Pain", "Difficult Breathing", "Severe Abdominal Pain", "Altered Mental Status", "High Fever with Convulsion",
    "Mild Fever", "Headache", "Nausea and Vomiting", "Sprain", "Common Cold", "Minor Rash"};
const string ZONES[] = {
    "RED", "RED", "RED", "RED", "RED",
    "YELLOW", "YELLOW", "YELLOW", "YELLOW", "YELLOW",
    "GREEN", "GREEN", "GREEN", "GREEN",
    "WHITE", "WHITE"};
const string LEVELS[] = {"Emergency", "Urgency", "Semi-urgency", "Non-urgency"};
const string MENU[] = {"Add Patient", "Serve Patient", "Show Queue", "History & Statistics", "Exit"};

// นับจำนวนสมาชิกของ SYMPTOMS[], LEVELS[], MENU[]
const int SYMPTOM_COUNT = sizeof(SYMPTOMS) / sizeof(SYMPTOMS[0]);
const int LEVEL_COUNT   = sizeof(LEVELS)   / sizeof(LEVELS[0]);
const int MENU_COUNT    = sizeof(MENU)     / sizeof(MENU[0]);

const int NO_PROMOTE_LEVEL = 2;   // level 1-2 ไม่เลื่อนขั้น
const int WAIT_LIMIT_MIN   = 30;  // รอเกินกี่นาทีถึงเลื่อน level
const int DEFAULT_PRIORITY = 11;  // priority เมื่อไม่พบอาการ

int priority[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
int n = sizeof(priority) / sizeof(priority[0]);

// check ว่า priority[], SYMPTOMS[], ZONES[] เท่ากัน
static_assert(sizeof(SYMPTOMS) / sizeof(SYMPTOMS[0]) == sizeof(ZONES) / sizeof(ZONES[0]), "SYMPTOMS and ZONES must have the same size");
static_assert(sizeof(priority) / sizeof(priority[0]) == sizeof(SYMPTOMS) / sizeof(SYMPTOMS[0]), "priority[] and SYMPTOMS must have the same size");

// ข้อมูลผู้ป่วย 1 คน
struct Patient{
    string name;
    string symptom; //เทียบกับ SYMPTOMS[]
    int level;
    int queueOrder; //ลำดับการมาถึง
    time_t arriveTime;
};

class Hospital{ //จัดการคิว
private:
    // คิวผู้ป่วย: array เดียว เรียงตามความสำคัญ (ตำแหน่ง 0 = คนถัดไป)
    Patient patients[MAX_SIZE];
    int patientCount = 0; //จน.ผู้ป่วยในคิว
    int orderCount = 0; //นับจน.คิว

    // ประวัติผู้ป่วยที่รักษาแล้ว (index เดียวกัน = คนเดียวกัน)
    string histNames[MAX_SIZE]; //เก็บสูงสุด 100 คน
    string histSymptoms[MAX_SIZE];
    int histLevels[MAX_SIZE];
    time_t histArriveTimes[MAX_SIZE];
    time_t histServeTimes[MAX_SIZE];
    int historyCount = 0; //นับจน.ประวัติที่รักษาแล้ว (ใช้ใน showHistory)

    int effectiveLevel(int i) const{
        int waitMinutes = (int)difftime(time(0), patients[i].arriveTime) / 60; //หาเวลาที่รอเป็นนาที
        if(patients[i].level <= NO_PROMOTE_LEVEL) return patients[i].level; //return level เดิม
        if(waitMinutes > WAIT_LIMIT_MIN) return patients[i].level - 1; //รอเกิน 30 นาที เลื่อนคิว
        return patients[i].level; //รอไม่เกิน 30 นาที return level เดิม
    }
    //check ว่า index อาการตรงกับอาการผู้ป่วยไหม
    int symptomIndex(int i) const{
        for(int k = 0; k < n; k++){ //k คือ index อาการ
            if(SYMPTOMS[k] == patients[i].symptom) return k;
        }
        return -1; //ไม่พบอาการ
    }
    int symptomPriority(int i) const{
        int k = symptomIndex(i);
        if(k == -1) return DEFAULT_PRIORITY; // ไม่พบอาการ
        return priority[k]; //พบอาการ
    }
    //check ว่าใครได้รักษาก่อน (เทียบผู้ป่วย 2 คน)
    bool isHighPriority(int a, int b) const{
        int levelA = effectiveLevel(a); //เรียก effectiveLevel เพื่อดู level
        int levelB = effectiveLevel(b);
        if(levelA != levelB) return levelA < levelB; //เทียบ level (return true เเล้วไม่ทำงานด้านล่าง)

        //level เท่ากันทำงานต่อ
        int priA = symptomPriority(a); //เรียก symptomPriority เพื่อดู priority อาการ
        int priB = symptomPriority(b);
        if(priA != priB) return priA < priB; //เทียบ priority อาการ (return true เเล้วไม่ทำงานด้านล่าง)

        //level และ priority อาการเท่ากัน
        return patients[a].queueOrder < patients[b].queueOrder; //เทียบลำดับการมาถึง
    }
    // เรียง patients[] ตามความสำคัญ (insertion sort สลับทั้งคนในครั้งเดียว)
    // ต้องเรียกก่อนใช้งานทุกครั้ง เพราะ effectiveLevel เปลี่ยนตามเวลา
    void sortQueue(){
        for(int i = 1; i < patientCount; i++){
            int j = i;
            while(j > 0 && isHighPriority(j, j - 1)){
                swap(patients[j], patients[j - 1]);
                j--;
            }
        }
    }
    // เพิ่มผู้ป่วยตำแหน่ง i ของคิว ลงประวัติ
    void addHistory(int i){
        if(historyCount >= MAX_SIZE){
            cout << "History is full" << endl;
            return;
        }
        histNames[historyCount] = patients[i].name;
        histSymptoms[historyCount] = patients[i].symptom;
        histLevels[historyCount] = patients[i].level;
        histArriveTimes[historyCount] = patients[i].arriveTime;
        histServeTimes[historyCount] = time(0);
        historyCount++;
    }
public:
    bool isFull() const{ return patientCount >= MAX_SIZE; }

    // เพิ่มผู้ป่วย
    void addPatient(string name, string symptom, int level){
        if(isFull()){
            cout << "Queue is full" << endl;
            return;
        }
        patients[patientCount] = {name, symptom, level, orderCount++, time(0)};
        patientCount++;
        cout << name << " => added to queue with level " << level << endl;
    }
    // รับข้อมูลจากผู้ป่วย + ตรวจสอบ
    void addPatientInput(){
        if(isFull()){
            cout << "Queue is full" << endl;
            return;
        }
        string name, symptom;
        int level, s;

        cout << "Enter patient name : ";
        while(true){
            getline(cin >> ws, name);
            bool invalidChar = false;

            for(int i = 0; i < (int)name.length(); i++){
                unsigned char c = (unsigned char)name[i];
                bool isEnglishLetter = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
                bool isSpace = (c == ' ');
                bool isThaiByte = (c >= 0x80);

                if(!(isEnglishLetter || isSpace || isThaiByte)){
                    invalidChar = true; break;
                }
            }
            if(name.empty() || invalidChar){
                cout << "** Please enter letters only **";
                cout << endl << "Enter patient name : ";
                continue;
            }
            break;
        }
        cout << endl << "========= Select patient symptom =========" << endl;
        for(int i = 0; i < SYMPTOM_COUNT; i++){
            cout << i + 1 << ". " << SYMPTOMS[i] << " [" << ZONES[i] << "]" << endl;
        }
        while(true){
            cout << endl << "Enter number (1-" << SYMPTOM_COUNT << "): ";

            string sInput;
            getline(cin, sInput);
            bool validDigits = !sInput.empty() && sInput.length() <= 9;

            for(int i = 0; i < (int)sInput.length(); i++){
                if(!isdigit((unsigned char)sInput[i])){
                    validDigits = false;
                    break;
                }
            }
            if(validDigits){
                s = stoi(sInput);
                if(s >= 1 && s <= SYMPTOM_COUNT) break;
            }
            cout << "** Invalid symptom number **" << endl;
        }
        symptom = SYMPTOMS[s - 1];

        while(true){
            cout << endl << "========================= Enter patient level =========================" << endl;
            for(int i = 0; i < LEVEL_COUNT; i++){
                cout << " " << i + 1 << "." << LEVELS[i];
                if(i < LEVEL_COUNT - 1) cout << "/";
            }
            cout << " : ";

            if(cin >> level && level >= 1 && level <= LEVEL_COUNT){
                string check;
                getline(cin, check);

                bool hasStrange = false;
                for(int i = 0; i < (int)check.length(); i++){
                    if(!isspace((unsigned char)check[i])){
                        hasStrange = true; break;
                    }
                }
                if(!hasStrange) break;
                cout << "** Invalid level, please enter level 1-" << LEVEL_COUNT << " **" << endl;
            }else{
                cout << "** Invalid level, please enter level 1-" << LEVEL_COUNT << " **" << endl;
                cin.clear();
                cin.ignore(1000, '\n');
            }
        }
        addPatient(name, symptom, level);
    }
    void showQueue(){ // แสดงคิวตามลำดับ ค.สำคัญ
        if(patientCount == 0){
            cout << "Queue is empty" << endl;
            return;
        }
        sortQueue();   // เรียง array จริงให้เป็นปัจจุบัน

        cout << endl << "=========== Show Queue Patients ===========" << endl;
        for(int r = 0; r < patientCount; r++){
            int k = symptomIndex(r);

            cout << "\n*** Serve Queue " << r + 1;
            if(r == 0) cout << " >>> NEXT";

            cout << endl << "(" << patients[r].name << ")"
                 << " Symptom is " << patients[r].symptom
                 << endl << "Effective level: " << effectiveLevel(r)
                 << " | Symptom priority: " << symptomPriority(r)
                 << " | Zone: " << (k == -1 ? "-" : ZONES[k])
                 << endl << "Arrive queue: " << patients[r].queueOrder + 1
                 << endl << "Time: " << ctime(&patients[r].arriveTime);
            cout << "-------------------------------------------";
        }
        cout << endl;
    }
    // เรียกผู้ป่วยเข้ารักษา
    void servePatient(){
        if(patientCount == 0){
            cout << "Queue is empty, no one to serve" << endl;
            return;
        }
        sortQueue();   // หลังเรียงแล้ว ตำแหน่ง 0 คือคนถัดไป

        cout << "=> Now patient: " << patients[0].name << " | Symptom: " << patients[0].symptom
             << " | Level " << patients[0].level << " (" << LEVELS[patients[0].level - 1] << ")" << endl;
        cout << "------------------------------------------------------------------------------------------------" << endl;

        // บันทึกลงประวัติ
        addHistory(0);

        // ลบออกจากคิว: เลื่อนสมาชิกถัดไปขึ้นมา (บรรทัดเดียวต่อคน)
        for(int i = 0; i < patientCount - 1; i++){
            patients[i] = patients[i + 1];
        }
        patientCount--;
    }
    // ประวัติ + สถิติ
    void showHistory(){
        cout << endl << "=========== PATIENT HISTORY & STATISTICS ===========" << endl;

        if(historyCount == 0){
            cout << "No patient history" << endl;
            return;
        }
        double totalTime = 0;
        for(int i = 0; i < historyCount; i++){
            cout << endl << "Patient " << i + 1 << endl;
            cout << "Name       : " << histNames[i] << endl;
            cout << "Symptom    : " << histSymptoms[i] << endl;
            cout << "Level      : " << histLevels[i] << " (" << LEVELS[histLevels[i] - 1] << ")" << endl;
            cout << "Arrive time: " << ctime(&histArriveTimes[i]);
            cout << "Serve time : " << ctime(&histServeTimes[i]);

            double waitTime = difftime(histServeTimes[i], histArriveTimes[i]);
            totalTime += waitTime;
            cout << "Waiting time : " << fixed << setprecision(2) << waitTime / 60.0 << " minute(s)" << endl;
        }
        double averageTime = totalTime / historyCount / 60.0;
        cout << endl << "Average waiting time : " << fixed << setprecision(2) << averageTime << " minute(s)" << endl;

        // นับผู้ป่วยแต่ละ level (index 1..LEVEL_COUNT)
        int levelCount[LEVEL_COUNT + 1] = {0};
        for(int i = 0; i < historyCount; i++){
            if(histLevels[i] >= 1 && histLevels[i] <= LEVEL_COUNT)
                levelCount[histLevels[i]]++;
        }
        cout << endl << "Patients treated by level" << endl;
        for(int i = 1; i <= LEVEL_COUNT; i++){
            cout << "Level " << i << " (" << LEVELS[i - 1] << ") : " << levelCount[i] << " patient(s)" << endl;
        }
        int maxLevel = 1;
        for(int i = 2; i <= LEVEL_COUNT; i++){
            if(levelCount[i] > levelCount[maxLevel]) maxLevel = i;
        }
        cout << endl << "Most treated level : Level " << maxLevel << " (" << LEVELS[maxLevel - 1] << ") - "
             << levelCount[maxLevel] << " patient(s)" << endl;
        cout << "=====================================================" << endl;
    }
};
int main(){
    Hospital h;
    string choice;

    cout << endl << "================================= Welcome to JubuJubu Hospital =================================" << endl;
    do{
        cout << endl << "(Choose number) ";
        for(int i = 0; i < MENU_COUNT; i++){
            cout << i + 1 << "." << MENU[i];
            if(i < MENU_COUNT - 1) cout << "/ ";
        }
        cout << ": ";

        getline(cin >> ws, choice);

        if(choice == "1"){
            h.addPatientInput();
        }else if(choice == "2"){
            h.servePatient();
        }else if(choice == "3"){
            h.showQueue();
        }else if(choice == "4"){
            h.showHistory();
        }else if(choice == "5"){
            cout << "Exited ..." << endl;
        }else{
            cout << "** Invalid number, please enter again **" << endl;
        }
    } while(choice != "5");
    return 0;
}