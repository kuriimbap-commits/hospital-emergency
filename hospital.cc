#include <iostream>
#include <string>
#include <ctime>
using namespace std;

// Cream
const int SYMPTOM_COUNT = 16;
const string SYMPTOMS[SYMPTOM_COUNT] = {
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
class Hospital{ //จัดการคิวผู้ป่วย
private:
    Patient *queue;
    int patientCount = 0; //จน.ผู้ป่วยในคิว
    int orderCount = 0;
    int capacity = 0; //ขนาดสูงสุดของ arr ตอนนี้

    void resizeQueue(){ //เพิ่มขนาดของ queue เป็น 2 เท่า
        int newCapacity = capacity * 2; //4 -> 8 -> 16
        Patient *newQueue = new Patient[newCapacity]; //ย้ายผู้ป่วยจาก queue เก่าไปยัง queue ใหม่
        for (int i=0; i<patientCount; i++){
            newQueue[i] = queue[i];
        } delete[] queue;
        queue = newQueue;
        capacity = newCapacity;
        cout << "[Queue resized to capacity " << capacity << "]" << endl;
    }
public:
    Hospital(){ //Constructor
        capacity = 4; //สร้าง new Patient[4]
        patientCount = 0;
        orderCount = 0;
        queue = new Patient[capacity];
    }
    ~Hospital(){ //Destructor
        delete[] queue;
    }
    void addPatient(string name, string symptom, int level){
        if (patientCount == capacity){ //คิวเต็ม เรียกใช้ resizeQueue() เพื่อเพิ่มขนาดของ queue
            resizeQueue(); //เพิ่มขนาดคิวเป็น *2
        }
        queue[patientCount].name = name; //ใส่ข้อมูลลง queue index ถัดไป
        queue[patientCount].symptom = symptom;
        queue[patientCount].level = level;
        queue[patientCount].queueOrder = orderCount++;
        queue[patientCount].arriveTime = time(0); //เก็บเวลาผู้ป่วยตอนเข้าคิว
        patientCount++;

        cout << name << " => added to queue with level " << level << endl;
        cout << "----------------------------------------------------------------------" << endl;
    }
    void addPatientInput(){ //รับข้อมูลผู้ป่วย
        string name, symptom;
        int level;
        int s;
        cout << "Enter patient name : ";
        while (true){
            getline(cin >> ws, name); //อ่านชื่อใหม่ทุกรอบ
            bool hasNumber = false;
            for (int i=0; i<name.length(); i++){
                if (name[i] >= '0' && name[i] <= '9'){
                hasNumber = true; break;
                }
            } if (hasNumber){
                cout << "Please enter name not number : "; continue;
            } break;
        }
        cout << endl << "--------- Select patient symptom ---------" << endl;

        for (int i = 0; i < SYMPTOM_COUNT; i++){
            cout << i+1 << ". " << SYMPTOMS[i] << endl;
        }
        while (true){
            cout << "Enter number (1-" << SYMPTOM_COUNT << "): ";
            if (cin >> s && s >= 1 && s <= SYMPTOM_COUNT){
                cin.ignore(1000, '\n'); break;
        }
            cout << "** Invalid symptom number **" << endl;
            cin.clear();
            cin.ignore(1000, '\n');
        }symptom = SYMPTOMS[s-1];

        while (true){ //check level input
            //1.วิกฤต, 2.ฉุกเฉิน, 3.ไม่รุนแรง, 4.ทั่วไป
            cout << endl << "------------------------- Enter patient level --------------------------" << endl << " 1.Emergency/ 2.Urgency/ 3.Semi-urgency/ 4.Non-urgency : ";
            if (cin >> level && level >=1 && level <=4){
                cin.ignore(1000, '\n'); break; //ignore ค่าที่ usr กรอกผิด
            } else{
                cout << "** Invalid level, please enter level 1-4 **" << endl;
                cin.clear(); //clear ค่าที่ usr กรอกผิดออก
                cin.ignore(1000, '\n');
            }
        } addPatient(name, symptom, level); //เก็บค่าที่กรอกถูกลง queue
    }
    void showQueue(){
        if (patientCount == 0){
            cout << "Queue is empty" << endl; return;
        }
        // สร้าง array เก็บ index แล้วเรียงตามลำดับที่จะถูกเรียกรักษา (ไม่แตะ queue จริง)
        int *order = new int[patientCount];
        for (int i = 0; i < patientCount; i++){
            order[i] = i;
        }
        // insertion sort โดยใช้ isHighPriority เป็นตัวเทียบ
        for (int i = 1; i < patientCount; i++){
            int key = order[i];
            Priority keyP(queue[key].level, queue[key].symptom, queue[key].queueOrder, queue[key].arriveTime);
            int j = i - 1;
            while (j >= 0){
                Priority other(queue[order[j]].level, queue[order[j]].symptom, queue[order[j]].queueOrder, queue[order[j]].arriveTime);
                if (keyP.isHighPriority(other)){
                    order[j + 1] = order[j];
                    j--;
                } else{ break; }
            } order[j + 1] = key;
    }
    // แสดงผลตามลำดับที่จะถูกเรียก
    cout << endl << "----------- Show Queue Patients -----------" << endl;
    for (int r = 0; r < patientCount; r++){
        Patient &p = queue[order[r]];
        Priority pr(p.level, p.symptom, p.queueOrder, p.arriveTime);

        cout << "\n*** Serve Queue " << r + 1;
        if (r == 0) cout << " >>> NEXT";
        cout << endl
             << "(" << p.name << ")" << " Symptom is " << p.symptom << endl << "Effective level: " << pr.getPriority() << " | Symptom rank: " << pr.inSeriousSysptom() << endl << "Arrive queue: " << p.queueOrder + 1 << endl << "Time: " << ctime(&p.arriveTime);
        cout << "-------------------------------------------";
    } cout << endl;
    delete[] order;
}
/// benzzz
class Priority {
private:
    int level;
    string sysptom;
    int queueOrder;
    time_t arriveTime;
public:
    // Constructor
    Priority(int level, string sysptom, int queueOrder, time_t arriveTime) {
        this->level = level;
        this->sysptom = sysptom;
        this->queueOrder = queueOrder;
        this->arriveTime = arriveTime;
    }
    //เลื่อนlevel
    int getPriority()const{
        //เวลาปัจจุบัน
        time_t currentTime=time(0);
        //เวลาที่รอ
        double waitTime = difftime(currentTime,arriveTime);
         int wiatMinutes=waitTime/60;
         if(level<=2){
            return level;
         }
         if (wiatMinutes>30)
         {
           return level-1;
         }
    return level;
    }
    // ดูความรุนแรงของอาการ
    // เลขน้อย = ฉุกเฉินกว่า
    int inSeriousSysptom() const {
        // RED
        if (sysptom == "Cardiac Arrest") {
            return 1;
        } else if (sysptom == "Not Breathing") {
            return 2;
        } else if (sysptom == "Unconsciousness") {
            return 3;
        } else if (sysptom == "Severe Shock") {
            return 4;
        } else if (sysptom == "Severe Bleeding") {
            return 5;
        } else if (sysptom == "Chest Pain") { // YELLOW
            return 6;
        } else if (sysptom == "Difficult Breathing") {
            return 7;
        } else if (sysptom == "Severe Abdominal Pain") {
            return 8;
        } else if (sysptom == "Altered Mental Status") {
            return 9;
        } else if (sysptom == "High Fever with Convulsion") {
            return 10;
        } else if (sysptom == "Mild Fever") { // GREEN
            return 11;
        } else if (sysptom == "Headache") {
            return 12;
        } else if (sysptom == "Nausea and Vomiting") {
            return 13;
        } else if (sysptom == "Sprain") {
            return 14;
        } else if (sysptom == "Common Cold") { // WHITE
            return 15;
        } else if (sysptom == "Minor Rash") {
            return 16;
        }
        // ไม่พบอาการ
        return 11;
    }
    // เปรียบเทียบ Priority
    //1. level 2. อาการ 3. ลำดับคิว
    bool isHighPriority(const Priority& other) const {
        int myLevel = getPriority();
        int otherLevel = other.getPriority();
        // 1. ดู Level
        if (myLevel < otherLevel) {
            return true;
        }
        if (myLevel > otherLevel) {
            return false;
        }
        // 2. Level เท่ากัน
        // ดูความฉุกเฉินของอาการ
        int mySysptomPriority = inSeriousSysptom();
        int otherSysptomPriority = other.inSeriousSysptom();

        if (mySysptomPriority < otherSysptomPriority) {
            return true;
        }
        if (mySysptomPriority > otherSysptomPriority) {
            return false;
        }
        // 3. Level และอาการเท่ากัน
        // ดูลำดับคิว
        return queueOrder < other.queueOrder;
    }
    // แสดง Priority
    void showPriority() {
        cout << "Level       : " << level << endl;
        cout << "Symptom     : " << sysptom << endl;
        cout << "Queue Order : " << queueOrder << endl;
    }
    // Getter
    int getLevel() const {
        return level;
    }
    string getSysptom() const {
        return sysptom;
    }
    int getQueueOrder() const {
        return queueOrder;
    }
};
    // penguin
    void servePatient(){
        if (patientCount == 0){
            cout << "Queue is empty, no one to serve" << endl;
            return;
        }
        //index ของคนที่ต้องรักษาก่อน (priority สูงสุด)
        int bestIndex = 0; //เริ่มสมมติว่าคนแรกสุดคือคนที่ priority สูงสุดก่อน
        for (int i = 1; i < patientCount; i++){
            Priority current(queue[i].level, queue[i].symptom, queue[i].queueOrder, queue[i].arriveTime);
            Priority best(queue[bestIndex].level, queue[bestIndex].symptom, queue[bestIndex].queueOrder, queue[bestIndex].arriveTime);
        if (current.isHighPriority(best)){
            bestIndex = i;
        }
        }
        //แสดงข้อมูลคนที่ถูกเรียกไปรักษา
        cout << "=> Now patient: " << queue[bestIndex].name << " | Symptom: " << queue[bestIndex].symptom << " | Level " << queue[bestIndex].level << endl;
        cout << "-----------------------------------------------------------------------------------" << endl;
        //ลบคนนี้ออกจาก array โดยเลื่อนสมาชิกที่เหลือขึ้นมาแทนที่ (ปิดช่องว่าง)
        for (int i = bestIndex; i < patientCount - 1; i++){
            queue[i] = queue[i + 1];
        } patientCount--; //ลดจำนวนผู้ป่วยในคิวลง 1
    }
};
int main(){
    Hospital h;
    string choice;
    cout << "----------------------- Welcome to JubuJubu Hospital -----------------------" << endl;
   do{
        // penguin
        cout << "(Choose number) 1.Add Patient/ 2.Serve Patient/ 3.Show Queue/ 4.Exit: ";// เพิ่ม Serve Patient เข้ามา
        getline(cin >> ws, choice);
        if (choice == "1"){
            h.addPatientInput();
        } else if (choice == "2"){
            h.servePatient(); //เรียกฟังก์ชัน Serve
        } else if (choice == "3"){
            h.showQueue();
        } else if (choice == "4"){ //เปลี่ยนเลขจาก 3 เป็น 4 แทรกเมนูใหม่เข้ามา
            cout << "Exited ..." << endl;
        } else{
            cout << "** Invalid number, please enter again **" << endl;
        }
    } while(choice != "4"); //เปลี่ยนเลขจาก 3 เป็น 4
    return 0;
}