#include <iostream>
#include <string>
#include <ctime>
#include <cctype>
#include <iomanip>
using namespace std;
 
// ขนาดของ array (ใช้ array [] ธรรมดา 1 มิติ ไม่ใช้ new/delete)
const int MAX = 100;          // ขนาดคิวผู้ป่วย
const int MAX_HISTORY = 100;  // ขนาดประวัติผู้ป่วย
 
// Cream
const string SYMPTOMS[] = {
    "Cardiac Arrest", "Not Breathing", "Unconsciousness",
    "Severe Shock", "Severe Bleeding", "Chest Pain",
    "Difficult Breathing", "Severe Abdominal Pain", "Altered Mental Status",
    "High Fever with Convulsion", "Mild Fever", "Headache",
    "Nausea and Vomiting", "Sprain", "Common Cold", "Minor Rash"
};
class Patient{
public:
    string name;
    string symptom; //อาการ
    int level = 0; //ค.รุนเเรง
    int queueOrder = 0; //ลำดับคิว
    time_t arriveTime = 0;
};
// เก็บข้อมูลผู้ป่วยที่รักษาแล้ว
class HistoryPatient{
public:
    string name;
    string symptom;
    int level = 0;
    time_t arriveTime = 0;
    time_t serveTime = 0;
};
//PRIORITY
class Priority{
private:
    int level;
    string sysptom;
    int queueOrder;
    time_t arriveTime;
 
public:
    // Constructor
    Priority(int level, string sysptom, int queueOrder, time_t arriveTime){
        this->level = level;
        this->sysptom = sysptom;
        this->queueOrder = queueOrder;
        this->arriveTime = arriveTime;
    }
    //เลื่อนlevel
    int getPriority()const{
        //เวลาปัจจุบัน
        time_t currentTime = time(0);
        //เวลาที่รอ
        double waitTime = difftime(currentTime, arriveTime);
        int wiatMinutes = waitTime / 60;
 
        if(level <= 2) return level;
        if(wiatMinutes > 30) return level - 1;
        return level;
    }
    // ดูความรุนแรงของอาการ
    // เลขน้อย = ฉุกเฉินกว่า
    int inSeriousSysptom() const{
        // RED
        if(sysptom == "Cardiac Arrest") return 1;
        else if(sysptom == "Not Breathing") return 2;
        else if(sysptom == "Unconsciousness") return 3;
        else if(sysptom == "Severe Shock") return 4;
        else if(sysptom == "Severe Bleeding") return 5;
        else if(sysptom == "Chest Pain") return 6; // YELLOW
        else if(sysptom == "Difficult Breathing") return 7;
        else if(sysptom == "Severe Abdominal Pain") return 8;
        else if(sysptom == "Altered Mental Status") return 9;
        else if(sysptom == "High Fever with Convulsion") return 10;
        else if(sysptom == "Mild Fever") return 11; // GREEN
        else if(sysptom == "Headache") return 12;
        else if(sysptom == "Nausea and Vomiting") return 13;
        else if(sysptom == "Sprain") return 14;
        else if(sysptom == "Common Cold") return 15; // WHITE
        else if(sysptom == "Minor Rash") return 16;
 
        // ไม่พบอาการ
        return 11;
    }
    // เปรียบเทียบ Priority
    //1. level 2. อาการ 3. ลำดับคิว
    bool isHighPriority(const Priority& other) const{
        int myLevel = getPriority();
        int otherLevel = other.getPriority();
 
        // 1. ดู Level
        if(myLevel < otherLevel) return true;
        if(myLevel > otherLevel) return false;
 
        // 2. Level เท่ากัน
        // ดูความฉุกเฉินของอาการ
        int mySysptomPriority = inSeriousSysptom();
        int otherSysptomPriority = other.inSeriousSysptom();
 
        if(mySysptomPriority < otherSysptomPriority) return true;
        if(mySysptomPriority > otherSysptomPriority) return false;
 
        // 3. Level และอาการเท่ากัน
        // ดูลำดับคิว
        return queueOrder < other.queueOrder;
    }
    // แสดง Priority
    void showPriority(){
        cout << "Level       : " << level << endl;
        cout << "Symptom     : " << sysptom << endl;
        cout << "Queue Order : " << queueOrder << endl;
    }
    // Getter
    int getLevel() const { return level; }
    string getSysptom() const { return sysptom; }
    int getQueueOrder() const { return queueOrder; }
};
class Hospital{ //จัดการคิวผู้ป่วย
private:
    // Priority Queue เก็บใน array 1 มิติ (index 0 = คนที่จะถูกเรียกก่อน)
    Patient queue[MAX];
    int patientCount = 0; //จน.ผู้ป่วยในคิว
    int orderCount = 0;
 
    // เก็บประวัติผู้ป่วยที่รักษาแล้ว (array 1 มิติ)
    HistoryPatient history[MAX_HISTORY];
    int historyCount = 0;
 
    // เรียง array queue ตาม priority (insertion sort ใน array เดิม)
    // เรียกก่อน show/serve ทุกครั้ง เพราะ level เปลี่ยนตามเวลาที่รอ
    void reorder(){
        for(int i = 1; i < patientCount; i++){
            Patient key = queue[i];
            Priority keyP(key.level, key.symptom, key.queueOrder, key.arriveTime);
            int j = i - 1;
            while(j >= 0){
                Priority other(queue[j].level, queue[j].symptom, queue[j].queueOrder, queue[j].arriveTime);
                if(keyP.isHighPriority(other)){
                    queue[j + 1] = queue[j];
                    j--;
                } else break;
            }
            queue[j + 1] = key;
        }
    }
 
    // เพิ่มข้อมูลลงประวัติ
    void addHistory(const Patient& p){
        if(historyCount >= MAX_HISTORY){
            cout << "History is full" << endl;
            return;
        }
        history[historyCount].name = p.name;
        history[historyCount].symptom = p.symptom;
        history[historyCount].level = p.level;
        history[historyCount].arriveTime = p.arriveTime;
        history[historyCount].serveTime = time(0);
        historyCount++;
    }
public:
    //ADD PATIENT
    void addPatient(string name, string symptom, int level){
        if(patientCount >= MAX){
            cout << "Queue is full" << endl;
            return;
        }
        queue[patientCount].name = name; //ใส่ข้อมูลลง queue index ถัดไป
        queue[patientCount].symptom = symptom;
        queue[patientCount].level = level;
        queue[patientCount].queueOrder = orderCount++;
        queue[patientCount].arriveTime = time(0); //เก็บเวลาผู้ป่วยตอนเข้าคิว
        patientCount++;
        reorder(); //จัดลำดับ priority ใน array
        cout << name << " => added to queue with level " << level << endl;
    }
    //ADD PATIENT
    void addPatientInput(){ //รับข้อมูลผู้ป่วย
        string name, symptom;
        int level, s;
 
        //แก้ตรงชื่อใส่อักขระพิเศษได้
        cout << "Enter patient name : ";
 
        while(true){
            getline(cin >> ws, name); //อ่านชื่อใหม่ทุกรอบ
            bool invalidChar = false;
 
            for(int i = 0; i < (int)name.length(); i++){
                unsigned char c = (unsigned char)name[i];
                bool isEnglishLetter = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
                bool isSpace = (c == ' ');
                bool isThaiByte = (c >= 0x80); //ตัวอักษรไทยใน UTF-8 จะมี byte แรก >= 0x80
 
                if(!(isEnglishLetter || isSpace || isThaiByte)){
                    invalidChar = true; break;
                }
            }
            if(name.empty() || invalidChar){
                cout << "** Please enter letters only **";
                cout << endl << "Enter patient name : ";
                continue;
            }break;
        }cout << endl << "========= Select patient symptom =========" << endl;
 
        int count = sizeof(SYMPTOMS)/sizeof(SYMPTOMS[0]);
        for(int i = 0; i < count; i++){
            cout << i + 1 << ". " << SYMPTOMS[i] << endl;
        }
        // แก้เวลารับตัวเลข "5-1", "3-", "56-25" ได้
        while(true){
            cout << endl << "Enter number (1-" << count << "): ";
 
            string sInput;
            getline(cin, sInput);
            bool validDigits = !sInput.empty() && sInput.length() <= 3;
 
            for(int i = 0; i < (int)sInput.length(); i++){
                if(!isdigit((unsigned char)sInput[i])){
                    validDigits = false;
                    break;
                }
            }
            if(validDigits){
                s = stoi(sInput); //แปลงข้อความเป็นตัวเลข หลังจากมั่นใจแล้วว่าเป็นตัวเลขล้วนๆ
                if(s >= 1 && s <= count) break; //ผ่านทุกเงื่อนไข ออกจาก loop
            }
            cout << "** Invalid symptom number **" << endl;
        }symptom = SYMPTOMS[s - 1];
 
        while(true){ //check level input
            //1.วิกฤต, 2.ฉุกเฉิน, 3.ไม่รุนแรง, 4.ทั่วไป
            cout << endl << "========================= Enter patient level =========================" << endl
                 << " 1.Emergency/ 2.Urgency/ 3.Semi-urgency/ 4.Non-urgency : ";
 
            if(cin >> level && level >= 1 && level <= 4){
                string check;
                getline(cin, check);
 
                bool hasStrange = false;
                for(int i=0; i<(int)check.length(); i++){
                    if(!isspace((unsigned char)check[i])){
                        hasStrange = true; break;
                    }
                }
                if(!hasStrange) break;
                cout << "** Invalid level, please enter level 1-4 **" << endl;
            }else{
                cout << "** Invalid level, please enter level 1-4 **" << endl;
                cin.clear(); //clear ค่าที่ usr กรอกผิดออก
                cin.ignore(1000, '\n');
            }
        }addPatient(name, symptom, level); //เก็บค่าที่กรอกถูกลง queue
    }
    //SHOW
    void showQueue(){
        if(patientCount == 0){
            cout << "Queue is empty" << endl;
            return;
        }
        reorder(); //จัดลำดับใน array ก่อนแสดง (index 0 = NEXT)
 
        // แสดงผลตามลำดับที่จะถูกเรียก
        cout << endl << "=========== Show Queue Patients ===========" << endl;
 
        for(int r = 0; r < patientCount; r++){
            Patient &p = queue[r];
            Priority pr(p.level, p.symptom, p.queueOrder, p.arriveTime);
 
            cout << "\n*** Serve Queue " << r + 1;
            if(r == 0) cout << " >>> NEXT";
 
            cout << endl << "(" << p.name << ")"
                 << " Symptom is " << p.symptom
                 << endl << "Effective level: " << pr.getPriority()
                 << " | Symptom rank: " << pr.inSeriousSysptom()
                 << endl << "Arrive queue: " << p.queueOrder + 1
                 << endl << "Time: " << ctime(&p.arriveTime);
            cout << "-------------------------------------------";
        }
        cout << endl;
    }
    //SERVE
    void servePatient(){
        if(patientCount == 0){
            cout << "Queue is empty, no one to serve" << endl;
            return;
        }
        reorder(); //คนที่ priority สูงสุดจะอยู่ index 0
 
        //แสดงข้อมูลคนที่ถูกเรียกไปรักษา
        cout << "=> Now patient: " << queue[0].name << " | Symptom: " << queue[0].symptom << " | Level " << queue[0].level << endl;
        cout << "------------------------------------------------------------------------------------------------" << endl;
 
        // เก็บข้อมูลผู้ป่วยก่อนลบออกจาก queue
        addHistory(queue[0]);
 
        //ลบคนแรกออกจาก array โดยเลื่อนสมาชิกที่เหลือขึ้นมาแทนที่ (ปิดช่องว่าง)
        for(int i = 0; i < patientCount - 1; i++){
            queue[i] = queue[i + 1];
        }patientCount--; //ลดจำนวนผู้ป่วยในคิวลง 1
    }
    // Function 5 : Patient History & Statistics
    // เก็บและแสดงประวัติผู้ป่วย
    void function5(){
        cout << endl << "=========== PATIENT HISTORY & STATISTICS ===========" << endl;
 
        if(historyCount == 0){
            cout << "No patient history" << endl;
            return;
        }
        // แสดงประวัติผู้ป่วยแต่ละคน
        for(int i = 0; i < historyCount; i++){
            cout << endl << "Patient " << i + 1 << endl;
            cout << "Name       : " << history[i].name << endl;
            cout << "Symptom    : " << history[i].symptom << endl;
            cout << "Level      : " << history[i].level << endl;
            cout << "Arrive time: " << ctime(&history[i].arriveTime);
            cout << "Serve time : " << ctime(&history[i].serveTime);
 
            double waitTime = difftime(history[i].serveTime, history[i].arriveTime);
            cout << "Waiting time : " << fixed << setprecision(2) << waitTime/60.0 << " minute(s)" << endl;
        }
        // คำนวณเวลาเฉลี่ย
        double totalTime = 0;
        double averageTime = 0;
        for(int i = 0; i < historyCount; i++){
            totalTime += difftime(history[i].serveTime, history[i].arriveTime);
        }
        averageTime = totalTime / historyCount / 60.0;
        cout << endl << "Average waiting time : " << fixed << setprecision(2) << averageTime << " minute(s)" << endl;
        // นับผู้ป่วยแต่ละ Level
        int levelCount[5] = {0};
        for(int i = 0; i < historyCount; i++){
            if(history[i].level >= 1 && history[i].level <= 4)
                levelCount[history[i].level]++;
        }cout << endl << "Patients treated by level" << endl;
 
        for(int i = 1; i <= 4; i++){
            cout << "Level " << i << " : " << levelCount[i] << " patient(s)" << endl;
        }
        // หา Level ที่มีผู้ป่วยถูกเรียกมารักษามากที่สุด
        int maxLevel = 1;
        for(int i = 2; i <= 4; i++){
            if(levelCount[i] > levelCount[maxLevel])
                maxLevel = i;
        }
        cout << endl << "Most treated level : Level "
             << maxLevel << " (" << levelCount[maxLevel]
             << " patient(s))" << endl;
        cout << "=====================================================" << endl;
    }
};
int main(){
    Hospital h;
    string choice;
 
    cout << endl << "================================= Welcome to JubuJubu Hospital =================================" << endl;
    do{
        cout << endl << "(Choose number) 1.Add Patient/ 2.Serve Patient/ 3.Show Queue/ 4.History & Statistics/ 5.Exit: ";
        getline(cin >> ws, choice);
 
        if(choice == "1"){
            h.addPatientInput();
        }else if(choice == "2"){
            h.servePatient(); //เรียกฟังก์ชัน Serve
        }else if(choice == "3"){
            h.showQueue();
        }else if(choice == "4"){
            // เรียกดูประวัติและสถิติผู้ป่วย
            h.function5();
        }else if(choice == "5"){
            cout << "Exited ..." << endl;
        }else{
            cout << "** Invalid number, please enter again **" << endl;
        }
    }while(choice != "5");
    return 0;
}