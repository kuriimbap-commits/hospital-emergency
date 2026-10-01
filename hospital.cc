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

// จำนวนสมาชิก คำนวณจาก array เอง (ไม่ต้องนับมือ)
const int SYMPTOM_COUNT = sizeof(SYMPTOMS) / sizeof(SYMPTOMS[0]);
const int LEVEL_COUNT   = sizeof(LEVELS)   / sizeof(LEVELS[0]);
const int MENU_COUNT    = sizeof(MENU)     / sizeof(MENU[0]);

const int NO_PROMOTE_LEVEL = 2;   // level 1-2 ไม่เลื่อนขั้น
const int WAIT_LIMIT_MIN   = 30;  // รอเกินกี่นาทีถึงเลื่อน level
const int DEFAULT_RANK     = 11;  // rank เมื่อไม่พบอาการ

// ตรวจตอนคอมไพล์ว่า SYMPTOMS กับ ZONES มีจำนวนเท่ากัน
static_assert(sizeof(SYMPTOMS) / sizeof(SYMPTOMS[0]) == sizeof(ZONES) / sizeof(ZONES[0]),
              "SYMPTOMS and ZONES must have the same size");

// ================= PRIORITY (ตัวเปรียบเทียบ สร้างชั่วคราวตอนเทียบ) =================
class Priority{
private:
    int level;
    string sysptom;
    int queueOrder;
    time_t arriveTime;
public:
    Priority(int level, string sysptom, int queueOrder, time_t arriveTime){
        this->level = level;
        this->sysptom = sysptom;
        this->queueOrder = queueOrder;
        this->arriveTime = arriveTime;
    }
    // เลื่อน level ถ้ารอนาน
    int getPriority() const{
        double waitTime = difftime(time(0), arriveTime);
        int waitMinutes = waitTime / 60;

        if(level <= NO_PROMOTE_LEVEL) return level;
        if(waitMinutes > WAIT_LIMIT_MIN) return level - 1;
        return level;
    }
    // ใช้ตำแหน่งใน SYMPTOMS[] เป็น rank (index 0 = rank 1, เลขน้อย = ฉุกเฉินกว่า)
    int inSeriousSysptom() const{
        for(int i = 0; i < SYMPTOM_COUNT; i++){
            if(SYMPTOMS[i] == sysptom) return i + 1;
        }
        return DEFAULT_RANK; // ไม่พบอาการ
    }
    // 1. level  2. อาการ  3. ลำดับคิว
    bool isHighPriority(const Priority& other) const{
        int myLevel = getPriority();
        int otherLevel = other.getPriority();
        if(myLevel < otherLevel) return true;
        if(myLevel > otherLevel) return false;

        int mySym = inSeriousSysptom();
        int otherSym = other.inSeriousSysptom();
        if(mySym < otherSym) return true;
        if(mySym > otherSym) return false;

        return queueOrder < other.queueOrder;
    }
};
class Hospital{
private:
    // คิวผู้ป่วย: index เดียวกัน = คนเดียวกัน
    string names[MAX_SIZE];
    string symptoms[MAX_SIZE];
    int levels[MAX_SIZE];
    int queueOrders[MAX_SIZE];
    time_t arriveTimes[MAX_SIZE];
    int patientCount = 0;
    int orderCount = 0;

    // ประวัติผู้ป่วยที่รักษาแล้ว
    string histNames[MAX_SIZE];
    string histSymptoms[MAX_SIZE];
    int histLevels[MAX_SIZE];
    time_t histArriveTimes[MAX_SIZE];
    time_t histServeTimes[MAX_SIZE];
    int historyCount = 0;

    // สร้าง Priority จากผู้ป่วยตำแหน่ง i ในคิว
    Priority makePriority(int i) const{
        return Priority(levels[i], symptoms[i], queueOrders[i], arriveTimes[i]);
    }

    // เพิ่มผู้ป่วยตำแหน่ง i ของคิว ลงประวัติ
    void addHistory(int i){
        if(historyCount >= MAX_SIZE){
            cout << "History is full" << endl;
            return;
        }
        histNames[historyCount] = names[i];
        histSymptoms[historyCount] = symptoms[i];
        histLevels[historyCount] = levels[i];
        histArriveTimes[historyCount] = arriveTimes[i];
        histServeTimes[historyCount] = time(0);
        historyCount++;
    }
public:
    bool isFull() const{ return patientCount >= MAX_SIZE; }

    void addPatient(string name, string symptom, int level){
        if(isFull()){
            cout << "Queue is full" << endl;
            return;
        }
        names[patientCount] = name;
        symptoms[patientCount] = symptom;
        levels[patientCount] = level;
        queueOrders[patientCount] = orderCount++;
        arriveTimes[patientCount] = time(0);
        patientCount++;
        cout << name << " => added to queue with level " << level << endl;
    }

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
                bool isThaiByte = (c >= 0x80); // ตัวอักษรไทยใน UTF-8 byte >= 0x80

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

    void showQueue(){
        if(patientCount == 0){
            cout << "Queue is empty" << endl;
            return;
        }
        // order[] เก็บ index เรียงตามลำดับที่จะถูกเรียก (ไม่แตะคิวจริง)
        int order[MAX_SIZE];
        for(int i = 0; i < patientCount; i++) order[i] = i;

        // insertion sort ใช้ isHighPriority เป็นตัวเทียบ
        for(int i = 1; i < patientCount; i++){
            int key = order[i];
            Priority keyP = makePriority(key);
            int j = i - 1;
            while(j >= 0){
                Priority other = makePriority(order[j]);
                if(keyP.isHighPriority(other)){
                    order[j + 1] = order[j];
                    j--;
                }else break;
            }
            order[j + 1] = key;
        }

        cout << endl << "=========== Show Queue Patients ===========" << endl;
        for(int r = 0; r < patientCount; r++){
            int p = order[r];
            Priority pr = makePriority(p);
            int idx = pr.inSeriousSysptom() - 1;

            cout << "\n*** Serve Queue " << r + 1;
            if(r == 0) cout << " >>> NEXT";

            cout << endl << "(" << names[p] << ")"
                 << " Symptom is " << symptoms[p]
                 << endl << "Effective level: " << pr.getPriority()
                 << " | Symptom rank: " << pr.inSeriousSysptom() << " | Zone: " << ZONES[idx]
                 << endl << "Arrive queue: " << queueOrders[p] + 1
                 << endl << "Time: " << ctime(&arriveTimes[p]);
            cout << "-------------------------------------------";
        }
        cout << endl;
    }

    void servePatient(){
        if(patientCount == 0){
            cout << "Queue is empty, no one to serve" << endl;
            return;
        }
        // หา index ของคนที่ priority สูงสุด
        int bestIndex = 0;
        for(int i = 1; i < patientCount; i++){
            Priority current = makePriority(i);
            Priority best = makePriority(bestIndex);
            if(current.isHighPriority(best)) bestIndex = i;
        }

        cout << "=> Now patient: " << names[bestIndex] << " | Symptom: " << symptoms[bestIndex]
             << " | Level " << levels[bestIndex] << " (" << LEVELS[levels[bestIndex] - 1] << ")" << endl;
        cout << "------------------------------------------------------------------------------------------------" << endl;

        addHistory(bestIndex);

        // ลบออกจากคิว: เลื่อนสมาชิกถัดไปขึ้นมา ทำครบทุก array
        for(int i = bestIndex; i < patientCount - 1; i++){
            names[i] = names[i + 1];
            symptoms[i] = symptoms[i + 1];
            levels[i] = levels[i + 1];
            queueOrders[i] = queueOrders[i + 1];
            arriveTimes[i] = arriveTimes[i + 1];
        }
        patientCount--;
    }

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