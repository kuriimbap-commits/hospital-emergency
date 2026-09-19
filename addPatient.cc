#include <iostream>
#include <string>
#include <ctime>
using namespace std;

// Cream
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
        cout << "Enter patient name : ";
        getline(cin >> ws, name); //ใช้ getline เพื่อรับชื่อที่เว้นวรรค, cin >> ws ข้ามช่องว่าง (getline จะได้ไม่อ่านส่วนที่เว้นเป็นช่องว่าง)
        cout << "Enter patient symptom : ";
        getline(cin >> ws, symptom);
        
        while (true){ //check level input
            //1.วิกฤต, 2.ฉุกเฉิน, 3.ไม่รุนแรง, 4.ทั่วไป
            cout << "Enter patient level is 1.Emergency/ 2.Urgency/ 3.Semi-urgency/ 4.Non-urgency : ";
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
        for(int i=0; i<patientCount; i++){
            cout << endl << "=> Person " << i+1 << endl << "(" << queue[i].name << ")" << " Symptom is " << queue[i].symptom << " |Level_" << queue[i].level << endl << "Time: " << ctime(&queue[i].arriveTime);
            cout << "----------------------------------";
        } cout << endl;
    }
};
int main(){
    Hospital h;
    string choice;
    cout << "-------------------- Welcome to JubuJubu Hospital --------------------" << endl;
   do{
        cout << "(Choose number) 1.Add Patient/ 2.Show Queue/ 3.Exit: ";
        getline(cin >> ws, choice);
        if (choice == "1"){
            h.addPatientInput();
        } else if (choice == "2"){
            h.showQueue();
        } else if (choice == "3"){
            cout << "Exited ..." << endl;
        } else{
            cout << "** Invalid number, please enter again **" << endl;
        }
    } while(choice != "3");
    return 0;
}