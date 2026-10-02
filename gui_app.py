#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
HỆ THỐNG GIÁM SÁT & BÁO CHÁY THÔNG MINH (STM32F103 FreeRTOS IoT Dashboard)
Giao diện Python PyQt5 - Điều khiển và giám sát 2 chiều qua UART JSON
"""

import sys
import json
import time
import serial
import serial.tools.list_ports
from datetime import datetime

from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
    QGridLayout, QLabel, QPushButton, QComboBox, QTextEdit, QLineEdit,
    QGroupBox, QFrame, QMessageBox, QGraphicsDropShadowEffect
)
from PyQt5.QtCore import Qt, QThread, pyqtSignal, QTimer
from PyQt5.QtGui import QFont, QColor, QIcon

# ==============================================================================
# STYLESHEET GIAO DIỆN DARK MODE CAO CẤP
# ==============================================================================
DARK_STYLE = """
QMainWindow {
    background-color: #0F172A;
}
QWidget {
    color: #E2E8F0;
    font-family: 'Segoe UI', 'Ubuntu', 'Helvetica Neue', sans-serif;
    font-size: 13px;
}
QGroupBox {
    background-color: #1E293B;
    border: 1px solid #334155;
    border-radius: 12px;
    margin-top: 24px;
    font-weight: bold;
    font-size: 14px;
    color: #38BDF8;
}
QGroupBox::title {
    subcontrol-origin: margin;
    subcontrol-position: top left;
    left: 16px;
    padding: 0 8px;
    background-color: #1E293B;
}
QComboBox {
    background-color: #0F172A;
    border: 1px solid #475569;
    border-radius: 8px;
    padding: 6px 12px;
    min-width: 140px;
    color: #F8FAFC;
    font-weight: 500;
}
QComboBox:hover {
    border: 1px solid #38BDF8;
}
QComboBox::drop-down {
    border: none;
    width: 24px;
}
QComboBox QAbstractItemView {
    background-color: #1E293B;
    border: 1px solid #475569;
    selection-background-color: #2563EB;
    color: #F8FAFC;
    padding: 4px;
}
QPushButton {
    background-color: #334155;
    border: 1px solid #475569;
    border-radius: 8px;
    padding: 8px 16px;
    font-weight: bold;
    color: #F8FAFC;
}
QPushButton:hover {
    background-color: #475569;
    border-color: #64748B;
}
QPushButton:pressed {
    background-color: #1E293B;
}
QPushButton#btnConnect {
    background-color: #059669;
    border: 1px solid #10B981;
}
QPushButton#btnConnect:hover {
    background-color: #10B981;
}
QPushButton#btnDisconnect {
    background-color: #DC2626;
    border: 1px solid #EF4444;
}
QPushButton#btnDisconnect:hover {
    background-color: #EF4444;
}
QPushButton#btnLedOn {
    background-color: #2563EB;
    border: 1px solid #3B82F6;
}
QPushButton#btnLedOn:hover {
    background-color: #3B82F6;
}
QPushButton#btnLedOff {
    background-color: #475569;
    border: 1px solid #64748B;
}
QPushButton#btnBuzzOn {
    background-color: #D97706;
    border: 1px solid #F59E0B;
}
QPushButton#btnBuzzOn:hover {
    background-color: #F59E0B;
}
QPushButton#btnBuzzOff {
    background-color: #475569;
    border: 1px solid #64748B;
}
QPushButton#btnAuto {
    background-color: #7C3AED;
    border: 1px solid #8B5CF6;
}
QPushButton#btnAuto:hover {
    background-color: #8B5CF6;
}
QTextEdit {
    background-color: #0A0F1D;
    border: 1px solid #1E293B;
    border-radius: 8px;
    padding: 8px;
    font-family: 'Consolas', 'Courier New', monospace;
    font-size: 12px;
    color: #A7F3D0;
}
QLineEdit {
    background-color: #0F172A;
    border: 1px solid #475569;
    border-radius: 8px;
    padding: 6px 12px;
    color: #F8FAFC;
    font-family: 'Consolas', 'Courier New', monospace;
}
QLineEdit:focus {
    border: 1px solid #38BDF8;
}
"""

# ==============================================================================
# THREAD ĐỌC SERIAL KHÔNG BLOCK GIAO DIỆN
# ==============================================================================
class SerialReaderThread(QThread):
    line_received = pyqtSignal(str)
    connection_lost = pyqtSignal(str)

    def __init__(self, ser):
        super().__init__()
        self.ser = ser
        self.is_running = True

    def run(self):
        while self.is_running and self.ser and self.ser.is_open:
            try:
                if self.ser.in_waiting > 0:
                    raw = self.ser.readline()
                    line = raw.decode('utf-8', errors='ignore').strip()
                    if line:
                        self.line_received.emit(line)
                else:
                    time.sleep(0.01)
            except Exception as e:
                if self.is_running:
                    self.connection_lost.emit(str(e))
                break

    def stop(self):
        self.is_running = False
        self.wait(500)


# ==============================================================================
# CỬA SỔ CHÍNH DASHBOARD
# ==============================================================================
class FireAlarmDashboard(QMainWindow):
    def __init__(self):
        super().__init__()
        self.ser = None
        self.reader_thread = None
        self.alarm_flashing = False
        self.flash_state = False

        self.init_ui()

        # Timer chớp đèn cảnh báo khi có lửa
        self.flash_timer = QTimer(self)
        self.flash_timer.timeout.connect(self.toggle_alarm_flash)

        # Quét cổng COM ban đầu
        self.refresh_ports()

    def init_ui(self):
        self.setWindowTitle("STM32F103 IoT Fire Alarm & Peripheral Controller")
        self.resize(1000, 720)
        self.setStyleSheet(DARK_STYLE)

        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        main_layout = QVBoxLayout(central_widget)
        main_layout.setContentsMargins(20, 16, 20, 20)
        main_layout.setSpacing(16)

        # 1. HEADER BAR
        header_layout = QHBoxLayout()
        title_box = QVBoxLayout()
        lbl_title = QLabel("🔥 HỆ THỐNG CẢNH BÁO CHÁY THÔNG MINH")
        lbl_title.setStyleSheet("font-size: 20px; font-weight: 800; color: #F8FAFC; letter-spacing: 0.5px;")
        lbl_sub = QLabel("STM32F103C8T6 • FreeRTOS Kernel • JSON UART Controller")
        lbl_sub.setStyleSheet("font-size: 12px; color: #94A3B8; font-weight: 500;")
        title_box.addWidget(lbl_title)
        title_box.addWidget(lbl_sub)
        header_layout.addLayout(title_box)

        header_layout.addStretch()

        self.lbl_conn_status = QLabel("● CHƯA KẾT NỐI")
        self.lbl_conn_status.setStyleSheet("""
            background-color: #334155;
            color: #94A3B8;
            font-weight: bold;
            font-size: 13px;
            padding: 8px 16px;
            border-radius: 20px;
        """)
        header_layout.addWidget(self.lbl_conn_status)
        main_layout.addLayout(header_layout)

        # 2. KHỐI KẾT NỐI SERIAL
        conn_group = QGroupBox("⚙️ Cấu Hình Cổng Serial")
        conn_layout = QHBoxLayout(conn_group)
        conn_layout.setContentsMargins(16, 16, 16, 16)
        conn_layout.setSpacing(12)

        conn_layout.addWidget(QLabel("Cổng COM / TTY:"))
        self.cb_ports = QComboBox()
        conn_layout.addWidget(self.cb_ports)

        self.btn_refresh = QPushButton("🔄 Quét lại")
        self.btn_refresh.clicked.connect(self.refresh_ports)
        conn_layout.addWidget(self.btn_refresh)

        conn_layout.addWidget(QLabel("Baudrate:"))
        self.cb_baud = QComboBox()
        self.cb_baud.addItems(["115200", "9600", "19200", "38400", "57600", "230400", "460800"])
        conn_layout.addWidget(self.cb_baud)

        self.btn_connect = QPushButton("⚡ Kết Nối")
        self.btn_connect.setObjectName("btnConnect")
        self.btn_connect.clicked.connect(self.toggle_connection)
        conn_layout.addWidget(self.btn_connect)

        conn_layout.addStretch()
        main_layout.addWidget(conn_group)

        # 3. KHỐI TRẠNG THÁI TRỰC QUAN (4 CARDS GRID)
        status_group = QGroupBox("📊 Giám Sát Thời Gian Thực")
        grid_status = QGridLayout(status_group)
        grid_status.setContentsMargins(16, 20, 16, 16)
        grid_status.setSpacing(16)

        # Card 1: Cảm biến lửa
        self.card_fire, self.lbl_fire_icon, self.lbl_fire_text = self.create_status_card(
            "CẢM BIẾN LỬA (FLAME)", "🛡️", "AN TOÀN", "#10B981"
        )
        grid_status.addWidget(self.card_fire, 0, 0)

        # Card 2: State Machine
        self.card_state, self.lbl_state_icon, self.lbl_state_text = self.create_status_card(
            "TRẠNG THÁI HỆ THỐNG", "⚙️", "IDLE (BÌNH THƯỜNG)", "#38BDF8"
        )
        grid_status.addWidget(self.card_state, 0, 1)

        # Card 3: Đèn LED
        self.card_led, self.lbl_led_icon, self.lbl_led_text = self.create_status_card(
            "ĐÈN BÁO (LED PB6)", "💡", "ĐANG TẮT", "#64748B"
        )
        grid_status.addWidget(self.card_led, 0, 2)

        # Card 4: Còi báo động
        self.card_buzz, self.lbl_buzz_icon, self.lbl_buzz_text = self.create_status_card(
            "CÒI BÁO ĐỘNG (BUZZER)", "🔔", "IM LẶNG", "#64748B"
        )
        grid_status.addWidget(self.card_buzz, 0, 3)

        main_layout.addWidget(status_group)

        # 4. KHỐI ĐIỀU KHIỂN & TERMINAL LOG (CHIA 2 CỘT)
        bottom_layout = QHBoxLayout()
        bottom_layout.setSpacing(16)

        # Cột trái: Điều khiển thủ công
        ctrl_group = QGroupBox("🎮 Bảng Điều Khiển Thiết Bị (JSON Commands)")
        ctrl_layout = QVBoxLayout(ctrl_group)
        ctrl_layout.setContentsMargins(16, 20, 16, 16)
        ctrl_layout.setSpacing(12)

        # Cài đặt thời lượng bật thủ công
        dur_layout = QHBoxLayout()
        dur_layout.addWidget(QLabel("⏱️ Thời lượng bật:"))
        self.cb_duration = QComboBox()
        self.cb_duration.addItem("Bật liên tục (Không hẹn giờ)", 0)
        self.cb_duration.addItem("1 Giây (1000ms)", 1000)
        self.cb_duration.addItem("2 Giây (2000ms)", 2000)
        self.cb_duration.addItem("3 Giây (3000ms)", 3000)
        self.cb_duration.addItem("5 Giây (5000ms)", 5000)
        self.cb_duration.addItem("10 Giây (10000ms)", 10000)
        self.cb_duration.setCurrentIndex(2) # Mặc định 2s
        dur_layout.addWidget(self.cb_duration)
        ctrl_layout.addLayout(dur_layout)

        # Hàng nút LED
        led_btn_layout = QHBoxLayout()
        self.btn_led_on = QPushButton("💡 Bật Đèn")
        self.btn_led_on.setObjectName("btnLedOn")
        self.btn_led_on.clicked.connect(self.handle_btn_led_on)
        self.btn_led_off = QPushButton("🌑 Tắt Đèn")
        self.btn_led_off.setObjectName("btnLedOff")
        self.btn_led_off.clicked.connect(lambda: self.send_json({"led": 0}))
        led_btn_layout.addWidget(self.btn_led_on)
        led_btn_layout.addWidget(self.btn_led_off)
        ctrl_layout.addLayout(led_btn_layout)

        # Hàng nút Còi
        buzz_btn_layout = QHBoxLayout()
        self.btn_buzz_on = QPushButton("🔔 Bật Còi")
        self.btn_buzz_on.setObjectName("btnBuzzOn")
        self.btn_buzz_on.clicked.connect(self.handle_btn_buzz_on)
        self.btn_buzz_off = QPushButton("🔕 Tắt Còi")
        self.btn_buzz_off.setObjectName("btnBuzzOff")
        self.btn_buzz_off.clicked.connect(lambda: self.send_json({"buzzer": 0}))
        buzz_btn_layout.addWidget(self.btn_buzz_on)
        buzz_btn_layout.addWidget(self.btn_buzz_off)
        ctrl_layout.addLayout(buzz_btn_layout)

        # Cấu hình báo cháy: Thời gian duy trì còi sau khi dập lửa
        alarm_cfg_layout = QHBoxLayout()
        alarm_cfg_layout.addWidget(QLabel("🔥 Còi hú sau dập lửa:"))
        self.cb_alarm_hold = QComboBox()
        self.cb_alarm_hold.addItem("1 Giây (1000ms)", 1000)
        self.cb_alarm_hold.addItem("2 Giây (2000ms)", 2000)
        self.cb_alarm_hold.addItem("3 Giây (3000ms)", 3000)
        self.cb_alarm_hold.addItem("5 Giây (5000ms)", 5000)
        self.cb_alarm_hold.addItem("10 Giây (10000ms)", 10000)
        self.cb_alarm_hold.setCurrentIndex(1) # Mặc định 2s
        self.cb_alarm_hold.currentIndexChanged.connect(self.handle_alarm_hold_changed)
        alarm_cfg_layout.addWidget(self.cb_alarm_hold)
        ctrl_layout.addLayout(alarm_cfg_layout)

        # Hàng nút Hệ Thống
        sys_btn_layout = QHBoxLayout()
        self.btn_auto = QPushButton("🤖 Chế Độ Tự Động")
        self.btn_auto.setObjectName("btnAuto")
        self.btn_auto.clicked.connect(lambda: self.send_json({"mode": "auto"}))
        self.btn_get_status = QPushButton("🔄 Đọc Trạng Thái")
        self.btn_get_status.clicked.connect(lambda: self.send_json({"get": "status"}))
        sys_btn_layout.addWidget(self.btn_auto)
        sys_btn_layout.addWidget(self.btn_get_status)
        ctrl_layout.addLayout(sys_btn_layout)

        ctrl_layout.addStretch()
        bottom_layout.addWidget(ctrl_group, 4)

        # Cột phải: Terminal UART Raw JSON Log
        log_group = QGroupBox("📝 Nhật Ký Giao Tiếp UART (JSON)")
        log_layout = QVBoxLayout(log_group)
        log_layout.setContentsMargins(16, 20, 16, 16)
        log_layout.setSpacing(8)

        self.txt_log = QTextEdit()
        self.txt_log.setReadOnly(True)
        log_layout.addWidget(self.txt_log)

        # Thanh nhập lệnh JSON tự do
        cmd_layout = QHBoxLayout()
        self.edit_cmd = QLineEdit()
        self.edit_cmd.setPlaceholderText('Nhập JSON tùy ý (vd: {"led":1, "buzzer":0})...')
        self.edit_cmd.returnPressed.connect(self.send_custom_command)
        self.btn_send_cmd = QPushButton("Gửi")
        self.btn_send_cmd.clicked.connect(self.send_custom_command)
        self.btn_clear_log = QPushButton("Xóa Log")
        self.btn_clear_log.clicked.connect(self.txt_log.clear)

        cmd_layout.addWidget(self.edit_cmd)
        cmd_layout.addWidget(self.btn_send_cmd)
        cmd_layout.addWidget(self.btn_clear_log)
        log_layout.addLayout(cmd_layout)

        bottom_layout.addWidget(log_group, 5)
        main_layout.addLayout(bottom_layout)

    def create_status_card(self, title, icon, text, color):
        frame = QFrame()
        frame.setStyleSheet(f"""
            QFrame {{
                background-color: #0F172A;
                border: 2px solid {color};
                border-radius: 12px;
                padding: 12px;
            }}
        """)
        layout = QVBoxLayout(frame)
        layout.setAlignment(Qt.AlignCenter)
        layout.setSpacing(6)

        lbl_t = QLabel(title)
        lbl_t.setStyleSheet("font-size: 11px; font-weight: bold; color: #94A3B8; text-transform: uppercase;")
        layout.addWidget(lbl_t, alignment=Qt.AlignCenter)

        lbl_i = QLabel(icon)
        lbl_i.setStyleSheet("font-size: 32px;")
        layout.addWidget(lbl_i, alignment=Qt.AlignCenter)

        lbl_val = QLabel(text)
        lbl_val.setStyleSheet(f"font-size: 14px; font-weight: 800; color: {color};")
        layout.addWidget(lbl_val, alignment=Qt.AlignCenter)

        return frame, lbl_i, lbl_val

    def refresh_ports(self):
        self.cb_ports.clear()
        ports = serial.tools.list_ports.comports()
        detected_index = 0
        for i, p in enumerate(ports):
            desc = f"{p.device} ({p.description})"
            self.cb_ports.addItem(desc, p.device)
            # Tự động ưu tiên chọn USB Serial (CH340, CP2102)
            if "USB" in p.device or "ACM" in p.device or "USB" in p.description:
                detected_index = i
        if ports:
            self.cb_ports.setCurrentIndex(detected_index)
        else:
            self.cb_ports.addItem("Không tìm thấy cổng nào", "")

    def toggle_connection(self):
        if self.ser and self.ser.is_open:
            self.disconnect_serial()
        else:
            self.connect_serial()

    def connect_serial(self):
        port = self.cb_ports.currentData()
        if not port:
            QMessageBox.warning(self, "Lỗi", "Vui lòng chọn cổng Serial hợp lệ!")
            return

        baud = int(self.cb_baud.currentText())
        try:
            self.ser = serial.Serial(port, baud, timeout=0.1)
            self.reader_thread = SerialReaderThread(self.ser)
            self.reader_thread.line_received.connect(self.handle_serial_line)
            self.reader_thread.connection_lost.connect(self.handle_connection_lost)
            self.reader_thread.start()

            self.btn_connect.setText("❌ Ngắt Kết Nối")
            self.btn_connect.setObjectName("btnDisconnect")
            self.btn_connect.setStyle(self.btn_connect.style())

            self.lbl_conn_status.setText(f"● ĐÃ KẾT NỐI ({port})")
            self.lbl_conn_status.setStyleSheet("""
                background-color: #064E3B;
                color: #34D399;
                font-weight: bold;
                font-size: 13px;
                padding: 8px 16px;
                border-radius: 20px;
                border: 1px solid #10B981;
            """)
            self.log_message(f"--- Đã kết nối thành công tới {port} ở {baud} baud ---", "#38BDF8")

            # Yêu cầu cập nhật trạng thái ngay
            self.send_json({"get": "status"})

        except Exception as e:
            QMessageBox.critical(self, "Lỗi kết nối", f"Không thể mở cổng {port}:\n{str(e)}")

    def disconnect_serial(self):
        if self.reader_thread:
            self.reader_thread.stop()
            self.reader_thread = None

        if self.ser:
            try:
                self.ser.close()
            except:
                pass
            self.ser = None

        self.btn_connect.setText("⚡ Kết Nối")
        self.btn_connect.setObjectName("btnConnect")
        self.btn_connect.setStyle(self.btn_connect.style())

        self.lbl_conn_status.setText("● CHƯA KẾT NỐI")
        self.lbl_conn_status.setStyleSheet("""
            background-color: #334155;
            color: #94A3B8;
            font-weight: bold;
            font-size: 13px;
            padding: 8px 16px;
            border-radius: 20px;
        """)
        self.stop_alarm_flash()
        self.log_message("--- Đã ngắt kết nối Serial ---", "#EF4444")

    def handle_connection_lost(self, err_msg):
        self.disconnect_serial()
        QMessageBox.warning(self, "Mất kết nối", f"Mất kết nối với thiết bị:\n{err_msg}")

    def handle_btn_led_on(self):
        dur = self.cb_duration.currentData()
        if dur > 0:
            self.send_json({"led": 1, "time": dur})
        else:
            self.send_json({"led": 1})

    def handle_btn_buzz_on(self):
        dur = self.cb_duration.currentData()
        if dur > 0:
            self.send_json({"buzzer": 1, "time": dur})
        else:
            self.send_json({"buzzer": 1})

    def handle_alarm_hold_changed(self):
        hold_ms = self.cb_alarm_hold.currentData()
        if hold_ms > 0:
            self.send_json({"alarm_hold": hold_ms})

    def send_json(self, data_dict):
        if not self.ser or not self.ser.is_open:
            QMessageBox.warning(self, "Chưa kết nối", "Vui lòng kết nối Serial trước khi điều khiển!")
            return

        json_str = json.dumps(data_dict) + "\n"
        try:
            self.ser.write(json_str.encode('utf-8'))
            self.log_message(f"[TX] {json_str.strip()}", "#38BDF8")
        except Exception as e:
            self.log_message(f"[TX LỖI] {str(e)}", "#EF4444")

    def send_custom_command(self):
        text = self.edit_cmd.text().strip()
        if not text:
            return
        if not text.endswith("\n"):
            text += "\n"
        if self.ser and self.ser.is_open:
            try:
                self.ser.write(text.encode('utf-8'))
                self.log_message(f"[TX] {text.strip()}", "#A78BFA")
                self.edit_cmd.clear()
            except Exception as e:
                self.log_message(f"[TX LỖI] {str(e)}", "#EF4444")
        else:
            QMessageBox.warning(self, "Chưa kết nối", "Vui lòng kết nối Serial trước!")

    def handle_serial_line(self, line):
        self.log_message(f"[RX] {line}", "#34D399")
        try:
            data = json.loads(line)
            self.update_telemetry(data)
        except json.JSONDecodeError:
            pass # Bỏ qua nếu là chuỗi log thông thường

    def update_telemetry(self, data):
        # 1. Trạng thái lửa
        if "fire" in data:
            fire = bool(data["fire"])
            raw_str = f" (PA1={data['raw']})" if "raw" in data else ""
            if fire:
                self.start_alarm_flash()
            else:
                self.stop_alarm_flash(raw_str)

        # 2. Trạng thái State Machine
        if "state" in data:
            st = str(data["state"]).upper()
            color = "#38BDF8"
            if st == "ALARM":
                color = "#EF4444"
                icon = "🔥"
            elif st == "VERIFYING":
                color = "#F59E0B"
                icon = "⏳"
            elif st == "MANUAL":
                color = "#A855F7"
                icon = "🎮"
            else:
                color = "#10B981"
                icon = "🛡️"

            self.lbl_state_icon.setText(icon)
            self.lbl_state_text.setText(st)
            self.lbl_state_text.setStyleSheet(f"font-size: 14px; font-weight: 800; color: {color};")
            self.card_state.setStyleSheet(f"""
                QFrame {{
                    background-color: #0F172A;
                    border: 2px solid {color};
                    border-radius: 12px;
                    padding: 12px;
                }}
            """)

        # 3. Trạng thái LED
        if "led" in data:
            led = bool(data["led"])
            color = "#FACC15" if led else "#64748B"
            self.lbl_led_icon.setText("💡" if led else "🌑")
            self.lbl_led_text.setText("BẬT (SÁNG)" if led else "ĐANG TẮT")
            self.lbl_led_text.setStyleSheet(f"font-size: 14px; font-weight: 800; color: {color};")
            self.card_led.setStyleSheet(f"""
                QFrame {{
                    background-color: #0F172A;
                    border: 2px solid {color};
                    border-radius: 12px;
                    padding: 12px;
                }}
            """)

        # 4. Trạng thái Còi
        if "buzzer" in data:
            buzz = bool(data["buzzer"])
            color = "#EF4444" if buzz else "#64748B"
            self.lbl_buzz_icon.setText("🔔" if buzz else "🔕")
            self.lbl_buzz_text.setText("ĐANG HÚ CÒI!" if buzz else "IM LẶNG")
            self.lbl_buzz_text.setStyleSheet(f"font-size: 14px; font-weight: 800; color: {color};")
            self.card_buzz.setStyleSheet(f"""
                QFrame {{
                    background-color: #0F172A;
                    border: 2px solid {color};
                    border-radius: 12px;
                    padding: 12px;
                }}
            """)

    def start_alarm_flash(self):
        if not self.alarm_flashing:
            self.alarm_flashing = True
            self.flash_timer.start(250)

    def stop_alarm_flash(self, extra=""):
        self.alarm_flashing = False
        self.flash_timer.stop()
        self.card_fire.setStyleSheet("""
            QFrame {
                background-color: #0F172A;
                border: 2px solid #10B981;
                border-radius: 12px;
                padding: 12px;
            }
        """)
        self.lbl_fire_icon.setText("🛡️")
        self.lbl_fire_text.setText(f"AN TOÀN{extra}")
        self.lbl_fire_text.setStyleSheet("font-size: 14px; font-weight: 800; color: #10B981;")

    def toggle_alarm_flash(self):
        self.flash_state = not self.flash_state
        bg = "#7F1D1D" if self.flash_state else "#0F172A"
        self.card_fire.setStyleSheet(f"""
            QFrame {{
                background-color: {bg};
                border: 3px solid #EF4444;
                border-radius: 12px;
                padding: 12px;
            }}
        """)
        self.lbl_fire_icon.setText("🔥" if self.flash_state else "⚠️")
        self.lbl_fire_text.setText("CẢNH BÁO CHÁY!")
        self.lbl_fire_text.setStyleSheet("font-size: 14px; font-weight: 800; color: #EF4444;")

    def log_message(self, msg, color_hex="#A7F3D0"):
        ts = datetime.now().strftime("%H:%M:%S.%f")[:-3]
        formatted = f'<span style="color:#64748B;">[{ts}]</span> <span style="color:{color_hex};">{msg}</span>'
        self.txt_log.append(formatted)


if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = FireAlarmDashboard()
    window.show()
    sys.exit(app.exec_())
