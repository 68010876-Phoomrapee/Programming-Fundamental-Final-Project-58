# 🚙 Program ระบบจัดการข้อมูลการตรวจสอบรถยนต์

## 🖥 วิธี Compile และ Run Program
### 📦 Files Required
  *ให้อยู่ใน Folder เดียวกัน*
  - **58_Project.c**
  
  - **users_data.csv**

### 1️⃣ รันจาก Terminal / Command Prompt

**Windows (ใช้ GCC ผ่าน MinGW)**
#### Compile
```bash 
gcc 58_Project.c -o 58_Project.exe
```

#### Run
```bash
.\58_Project.exe
```

### 2️⃣ สร้างและรันไฟล์ EXE (Windows)
เปิด Command Prompt หรือ PowerShell

ไปยัง Folder Program


Compile ด้วย GCC:
```bash 
gcc 58_Project.c -o 58_Project.exe
```
จะได้ไฟล์ 58_Project.exe

Run โดยการดับเบิ้ลคลิกไฟล์ 
หรือเรียกจาก Command Prompt:
```bash 
.\58_Project.exe
```
### 3️⃣ รันโปรแกรมใน VS Code Terminal
เปิด VS Code → เปิด Folder Program

เปิด Terminal

Compile:

#### Windows
```bash 
gcc 58_Project.c -o 58_Project.exe
```
#### Linux/macOS
```bash 
gcc 58_Project.c -o 58_Project.out
```
Run:
#### Windows
```bash 
.\58_Project.exe
```
#### Linux/macOS
```bash 
./58_Project.out
```

---

## 📁 โครงสร้างไฟล์
├── 58_Project.c                     # Code หลักของโปรแกรม

├── Project.h                        # Header Source ไฟล์ของโปรแกรม

├── users_data.csv                  # ไฟล์เก็บข้อมูลผู้ใช้งาน

├── Unit_Test.c                     # ไฟล์ Unit Test

└── E2E_Test.c                      # ไฟล์ E2E Test

---

## 💾 ระบบไฟล์ CSV: `users_data.csv`

ไฟล์ CSV เก็บข้อมูลผู้ใช้งาน มี **4 คอลัมน์** ดังนี้:

| คอลัมน์          | รูปแบบตัวอย่าง |
|------------------|----------------|
| InspectionID      | ตัวอักษรใหญ่ 1 ตัว + ตัวเลข 3 ตัว (เช่น A001) |
| CarRegNumber      | ตัวอักษรใหญ่ 3 ตัว + ตัวเลข 4 ตัว (เช่น ABC1234) |
| OwnerName         | ตัวอักษรและเว้นวรรค (เช่น John Doe) |
| InspectionDate    | DD/MM/YYYY (เช่น 01/08/2025) |

> หากไฟล์ CSV ไม่มีอยู่ โปรแกรมจะสร้าง **ตัวอย่างข้อมูล 20 record อัตโนมัติ**

---

## 💻 ฟีเจอร์หลัก

- **Add Record** – เพิ่มข้อมูลการตรวจสอบรถยนต์  
- **Search Record** – ค้นหาโดย **InspectionID** หรือ **CarRegNumber**  
- **Update Record** – แก้ไขข้อมูลที่มีอยู่ โดยค้นหาจาก **InspectionID** หรือ **CarRegNumber**  
- **Delete Record** – ลบข้อมูล โดยค้นหาจาก **InspectionID** หรือ **CarRegNumber**  
- **Unit Tests** – ทดสอบฟังก์ชัน **Search** และ **Delete**  
- **E2E Test** – ทดสอบระบบครบวงจร (**Add → Search → Update → Delete**)  
- **Exit** – ออกจากโปรแกรม  

---

## 📝 ฟังก์ชันเสริม

- **Validation** – ตรวจสอบรูปแบบ:  
  - `InspectionID` ✅  
  - `CarRegNumber` ✅  
  - `OwnerName` ✅  
  - `InspectionDate` ✅  

- **Confirm Action** – ยืนยันก่อน **Add / Update / Delete**  
- **Clear Screen** – รองรับ **Windows (cls)** และ **Linux/macOS (clear)**  

---
