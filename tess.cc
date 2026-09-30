#include <iostream>
#include <string>
#include <ctime>
#include <cctype>
#include <iomanip>
using namespace std;
 
// ================= ค่าคงที่ =================
const int MAX = 100;           // ขนาด array จริงของคิว
const int MAX_HISTORY = 1000;  // ขนาด array จริงของประวัติ
 
const int SYMPTOM_COUNT = 16;
const string SYMPTOMS[SYMPTOM_COUNT] = {
    "Cardiac Arrest", "Not Breathing", "Unconsciousness",
    "Severe Shock", "Severe Bleeding", "Chest Pain",
    "Difficult Breathing", "Severe Abdominal Pain", "Altered Mental Status",
    "High Fever with Convulsion", "Mild Fever", "Headache",
    "Nausea and Vomiting", "Sprain", "Common Cold", "Minor Rash"
};
 
// เวลาจำลอง: ใช้ทดสอบ Time base (เดินเวลาข้างหน้าโดยไม่ต้องรอจริง)
long timeOffset = 0;
time_t nowTime() { return time(0) + timeOffset; }
 
string levelName(int l) {
    if (l == 1) return "Critical";
    if (l == 2) return "Emergency";
    if (l == 3) return "Urgent";
    return "Non-urgent";
}
 
// อ่านตัวเลขในช่วง lo-hi (รับเฉพาะตัวเลขล้วน กัน "5-1", "3-", "56-25")
int readNumber(const string& prompt, int lo, int hi, const string& err) {
    while (true) {
        cout << prompt;
        string s;
        getline(cin >> ws, s);
        bool ok = !s.empty() && s.length() <= 3;
        for (int i = 0; i < (int)s.length(); i++) {
            if (!isdigit((unsigned char)s[i])) { ok = false; break; }
        }
        if (ok) {
            int v = stoi(s);
            if (v >= lo && v <= hi) return v;
        }
        cout << err << endl;
    }
}
 
class Patient {
public:
    string name;
    string symptom;
    int level = 0;        // ค.รุนแรง (level ตอนเข้าคิว)
    int queueOrder = 0;   // ลำดับการเข้าคิว (ใช้ FIFO)
    time_t arriveTime = 0;
};
 
// เก็บข้อมูลผู้ป่วยที่รักษาแล้ว
class HistoryPatient {
public:
    string name;
    string symptom;
    int level = 0;
    time_t arriveTime = 0;
    time_t serveTime = 0;
};
 
// ================= PRIORITY =================
class Priority {
private:
    int level;
    int queueOrder;
    time_t arriveTime;
 
public:
    Priority(int level, int queueOrder, time_t arriveTime) {
        this->level = level;
        this->queueOrder = queueOrder;
        this->arriveTime = arriveTime;
    }
 
    // Time base: รอครบทุก 30 นาที ขยับขึ้น 1 level
    // Level 1,2 ไม่ boost / Level 3,4 ขึ้นได้สูงสุดแค่ Level 2
    int getPriority() const {
        if (level <= 2) return level;
        int waitMinutes = (int)(difftime(nowTime(), arriveTime) / 60);
        int boosted = level - waitMinutes / 30;
        if (boosted < 2) boosted = 2;
        return boosted;
    }
 
    // เปรียบเทียบ: 1. level (น้อยก่อน)  2. level เท่ากัน -> FIFO
    bool isHighPriority(const Priority& other) const {
        int myLevel = getPriority();
        int otherLevel = other.getPriority();
        if (myLevel != otherLevel) return myLevel < otherLevel;
        return queueOrder < other.queueOrder;
    }
};
 
// ================= HOSPITAL =================
class Hospital {
private:
    // Priority Queue เก็บใน array 1 มิติ (ไม่ใช้ new[])
    Patient queue[MAX];
    int patientCount = 0;
    int orderCount = 0;
    int capacity = 4;   // ความจุ "จำลอง" เริ่ม 4 -> 8 -> 16 ... (ไม่เกิน MAX)
 
    HistoryPatient history[MAX_HISTORY];
    int historyCount = 0;
 
    // เรียง array ตาม priority (index 0 = คนที่จะถูกเรียกก่อน)
    // ต้องเรียกใหม่ทุกครั้งก่อน show/serve เพราะ level เปลี่ยนตามเวลา
    void reorder() {
        for (int i = 1; i < patientCount; i++) {
            Patient key = queue[i];
            Priority keyP(key.level, key.queueOrder, key.arriveTime);
            int j = i - 1;
            while (j >= 0) {
                Priority other(queue[j].level, queue[j].queueOrder, queue[j].arriveTime);
                if (keyP.isHighPriority(other)) {
                    queue[j + 1] = queue[j];
                    j--;
                } else break;
            }
            queue[j + 1] = key;
        }
    }
 
    void addHistory(const Patient& p) {
        if (historyCount == MAX_HISTORY) {
            cout << "(History is full, this record is not saved)" << endl;
            return;
        }
        history[historyCount].name = p.name;
        history[historyCount].symptom = p.symptom;
        history[historyCount].level = p.level;
        history[historyCount].arriveTime = p.arriveTime;
        history[historyCount].serveTime = nowTime();
        historyCount++;
    }
 
    void removeAt(int pos) {
        for (int i = pos; i < patientCount - 1; i++) queue[i] = queue[i + 1];
        patientCount--;
    }
 
public:
    // ADD PATIENT
    void addPatient(string name, string symptom, int level) {
        if (patientCount == MAX) {
            cout << "Queue is full (" << MAX << "), cannot add more patients" << endl;
            return;
        }
        if (patientCount == capacity) {   // จำลองการ resize x2
            int oldCap = capacity;
            capacity *= 2;
            if (capacity > MAX) capacity = MAX;
            cout << "[Resize] capacity " << oldCap << " -> " << capacity << endl;
        }
        queue[patientCount].name = name;
        queue[patientCount].symptom = symptom;
        queue[patientCount].level = level;
        queue[patientCount].queueOrder = orderCount++;
        queue[patientCount].arriveTime = nowTime();
        patientCount++;
        reorder();
        cout << name << " => added to queue with level " << level
             << " (" << levelName(level) << ")" << endl;
    }
 
    void addPatientInput() {
        string name, symptom;
 
        cout << "Enter patient name : ";
        while (true) {
            getline(cin >> ws, name);
            bool invalidChar = false;
            for (int i = 0; i < (int)name.length(); i++) {
                unsigned char c = (unsigned char)name[i];
                bool isEnglishLetter = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
                bool isSpace = (c == ' ');
                bool isThaiByte = (c >= 0x80);
                if (!(isEnglishLetter || isSpace || isThaiByte)) { invalidChar = true; break; }
            }
            if (name.empty() || invalidChar) {
                cout << "** Please enter letters only **" << endl << "Enter patient name : ";
                continue;
            }
            break;
        }
 
        cout << endl << "========= Select patient symptom =========" << endl;
        for (int i = 0; i < SYMPTOM_COUNT; i++)
            cout << i + 1 << ". " << SYMPTOMS[i] << endl;
        int s = readNumber("\nEnter number (1-16): ", 1, SYMPTOM_COUNT, "** Invalid symptom number **");
        symptom = SYMPTOMS[s - 1];
 
        cout << endl << "========================= Enter patient level =========================" << endl;
        int level = readNumber(" 1.Critical/ 2.Emergency/ 3.Urgent/ 4.Non-urgent : ", 1, 4,
                               "** Invalid level, please enter level 1-4 **");
 
        addPatient(name, symptom, level);
    }
 
    // SHOW
    void showQueue() {
        if (patientCount == 0) {
            cout << "Queue is empty" << endl;
            return;
        }
        reorder();
        cout << endl << "=========== Show Queue Patients ===========" << endl;
        cout << "In queue: " << patientCount << " | capacity: " << capacity << endl;
 
        for (int i = 0; i < patientCount; i++) {
            Patient& p = queue[i];
            Priority pr(p.level, p.queueOrder, p.arriveTime);
            int waitMin = (int)(difftime(nowTime(), p.arriveTime) / 60);
 
            cout << "\n*** Serve Queue " << i + 1;
            if (i == 0) cout << " >>> NEXT";
            cout << endl << "(" << p.name << ") Symptom is " << p.symptom
                 << endl << "Level: " << p.level << " -> Effective level: " << pr.getPriority()
                 << " (" << levelName(pr.getPriority()) << ")"
                 << endl << "Waiting: " << waitMin << " minute(s)"
                 << endl << "Time: " << ctime(&p.arriveTime);
            cout << "-------------------------------------------";
        }
        cout << endl;
    }
 
    // SERVE (Dequeue คนที่ priority สูงสุด = index 0)
    void servePatient() {
        if (patientCount == 0) {
            cout << "Queue is empty, no one to serve" << endl;
            return;
        }
        reorder();
        Patient p = queue[0];
        Priority pr(p.level, p.queueOrder, p.arriveTime);
        cout << "=> Now patient: " << p.name << " | Symptom: " << p.symptom
             << " | Level " << p.level << " (effective " << pr.getPriority() << ")" << endl;
        cout << "------------------------------------------------------------------------" << endl;
 
        addHistory(p);
        removeAt(0);
    }
 
    // REMOVE (ผู้ป่วยยกเลิก / ย้าย รพ.)
    void removePatient() {
        if (patientCount == 0) {
            cout << "Queue is empty, nobody to remove" << endl;
            return;
        }
        string name;
        cout << "Enter patient name to remove : ";
        getline(cin >> ws, name);
        for (int i = 0; i < patientCount; i++) {
            if (queue[i].name == name) {
                removeAt(i);
                cout << name << " => removed from queue" << endl;
                return;
            }
        }
        cout << "Patient \"" << name << "\" not found" << endl;
    }
 
    // ทดสอบ Time base โดยเดินเวลาจำลอง
    void advanceTime() {
        int m = readNumber("Advance time by how many minutes (1-999): ", 1, 999, "** Invalid number **");
        timeOffset += (long)m * 60;
        cout << "Time advanced " << m << " minute(s)" << endl;
    }
 
    // HISTORY & STATISTICS
    void showHistory() {
        cout << endl << "=========== PATIENT HISTORY & STATISTICS ===========" << endl;
        if (historyCount == 0) {
            cout << "No patient history" << endl;
            return;
        }
        double totalTime = 0;
        int levelCount[5] = {0, 0, 0, 0, 0};
        double levelWait[5] = {0, 0, 0, 0, 0};
 
        for (int i = 0; i < historyCount; i++) {
            double w = difftime(history[i].serveTime, history[i].arriveTime);
            cout << endl << "Patient " << i + 1 << endl;
            cout << "Name       : " << history[i].name << endl;
            cout << "Symptom    : " << history[i].symptom << endl;
            cout << "Level      : " << history[i].level << endl;
            cout << "Arrive time: " << ctime(&history[i].arriveTime);
            cout << "Serve time : " << ctime(&history[i].serveTime);
            cout << "Waiting time : " << fixed << setprecision(2) << w / 60.0 << " minute(s)" << endl;
 
            totalTime += w;
            levelCount[history[i].level]++;
            levelWait[history[i].level] += w;
        }
        cout << endl << "Average waiting time : " << fixed << setprecision(2)
             << totalTime / historyCount / 60.0 << " minute(s)" << endl;
 
        cout << endl << "Patients treated by level" << endl;
        int maxLevel = 1;
        for (int i = 1; i <= 4; i++) {
            cout << "Level " << i << " (" << levelName(i) << ") : " << levelCount[i] << " patient(s)";
            if (levelCount[i] > 0)
                cout << " | avg wait " << fixed << setprecision(2)
                     << levelWait[i] / levelCount[i] / 60.0 << " minute(s)";
            cout << endl;
            if (levelCount[i] > levelCount[maxLevel]) maxLevel = i;
        }
        cout << endl << "Most treated level : Level " << maxLevel
             << " (" << levelCount[maxLevel] << " patient(s))" << endl;
        cout << "=====================================================" << endl;
    }
};
 
int main() {
    Hospital h;
    string choice;
 
    cout << endl << "================================= Welcome to JubuJubu Hospital =================================" << endl;
    do {
        cout << endl << "(Choose number) 1.Add Patient/ 2.Serve Patient/ 3.Show Queue/ 4.Remove Patient/"
             << " 5.History & Statistics/ 6.Advance Time (test)/ 7.Exit: ";
        getline(cin >> ws, choice);
 
        if (choice == "1") h.addPatientInput();
        else if (choice == "2") h.servePatient();
        else if (choice == "3") h.showQueue();
        else if (choice == "4") h.removePatient();
        else if (choice == "5") h.showHistory();
        else if (choice == "6") h.advanceTime();
        else if (choice == "7") cout << "Exited ..." << endl;
        else cout << "** Invalid number, please enter again **" << endl;
    } while (choice != "7");
    return 0;
}