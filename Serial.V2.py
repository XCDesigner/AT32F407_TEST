import tkinter as tk
from tkinter import ttk
import serial
import serial.tools.list_ports
import time

ser = None  # 串口对象

def refresh_serial_ports():
    ports = serial.tools.list_ports.comports()
    combo['values'] = [p.device for p in ports]
    if combo['values']:
        combo.current(0)
    else:
        combo.set('无可用串口')

def open_serial():
    global ser
    port = combo.get()
    if port and port != '无可用串口':
        try:
            ser = serial.Serial(port, 115200, timeout=0.1)
            status.set(f"已连接 {port}")
        except Exception as e:
            status.set(f"连接失败: {e}")
    else:
        status.set("无可用串口")

def send_command(cmd):
    """只发送，不在界面打印命令"""
    if ser and ser.is_open:
        try:
            ser.write((cmd + '\r\n').encode())
            status.set("已发送指令")
        except Exception as e:
            status.set(f"发送失败: {e}")
    else:
        status.set("串口未连接")

def read_serial():
    """持续读取并显示收到的数据"""
    if ser and ser.is_open:
        try:
            while ser.in_waiting:
                data = ser.readline().decode('gbk', errors='ignore')
                if data:
                    text_area.insert(tk.END, data)
                    text_area.see(tk.END)
        except Exception:
            pass
    root.after(100, read_serial)

def send_pwm_duty():
    cmd = f"set pwm {entry_tmr_duty.get()} ch {entry_ch_duty.get()} duty {entry_duty.get()}"
    send_command(cmd)

def send_pwm_freq():
    cmd = f"set pwm {entry_tmr_freq.get()} ch {entry_ch_freq.get()} fre {entry_freq.get()}"
    send_command(cmd)

def send_dac():
    cmd = f"set dac {entry_dac_num.get()} {entry_dac_val.get()}"
    send_command(cmd)

def query_temperature():
    if not (ser and ser.is_open):
        status.set("串口未连接")
        return

    ser.reset_input_buffer()
    for ch in range(1, 13): #通道1-12
        cmd = f"print adc 1 ch {ch}"
        ser.write((cmd + '\r\n').encode())
        line = ser.readline().decode('gbk', errors='ignore')
        if line:
            text_area.insert(tk.END, line)
            text_area.see(tk.END)
        time.sleep(0.05)

    status.set("温度查询完成")

def query_status():
    send_command("cfg")

def help_example():
    send_command("help")

def clear_text():
    """清空回显区"""
    text_area.delete('1.0', tk.END)
    status.set("已清空回显")

def shutdown_all():
    """一键关停：关闭 PWM8 的 1~4 通道"""
    if not (ser and ser.is_open):
        status.set("串口未连接")
        return

    cmds = [
        "clear",
    ]
    for c in cmds:
        send_command(c)
        time.sleep(0.02)  # 轻微间隔，避免设备侧粘包或处理不过来

    status.set("一键关停完成")

def restore():
    """恢复PWM8 的 1~4 通道上一次配置的Duty"""
    if not (ser and ser.is_open):
        status.set("串口未连接")
        return

    cmds = [
        "restore pwm 8",
    ]
    for c in cmds:
        send_command(c)
        time.sleep(0.02)  # 轻微间隔，避免设备侧粘包或处理不过来

    status.set("恢复完成")



# 创建主窗口
root = tk.Tk()
root.title('串口控制应用')
root.geometry('600x600')

status = tk.StringVar(value="未连接串口")

# 串口选择区域
combo = ttk.Combobox(root, state='readonly')
combo.pack(fill='x', padx=10, pady=5)
ttk.Button(root, text='刷新串口列表', command=refresh_serial_ports).pack(padx=10, pady=2)
ttk.Button(root, text='打开串口', command=open_serial).pack(padx=10, pady=2)
ttk.Label(root, textvariable=status).pack(padx=10, pady=2)

# PWM Duty 控制
ttk.Label(root, text="PWM Duty 控制").pack(pady=5)
frame_duty = ttk.Frame(root)
frame_duty.pack(fill='x', padx=10, pady=2)
entry_tmr_duty = ttk.Entry(frame_duty, width=5); entry_tmr_duty.insert(0, "8")
entry_ch_duty = ttk.Entry(frame_duty, width=5); entry_ch_duty.insert(0, "4")
entry_duty = ttk.Entry(frame_duty, width=10); entry_duty.insert(0, "20")
for w in (entry_tmr_duty, entry_ch_duty, entry_duty):
    w.pack(side='left', padx=2)
ttk.Button(frame_duty, text='发送Duty', command=send_pwm_duty).pack(side='left', padx=5)
ttk.Button(frame_duty, text='一键关停', command=shutdown_all).pack(side='left', padx=5)
ttk.Button(frame_duty, text='恢复上一次PWM(上电默认100%)', command=restore).pack(side='left', padx=5)

# PWM Frequency 控制
ttk.Label(root, text="PWM Frequency 控制").pack(pady=5)
frame_freq = ttk.Frame(root)
frame_freq.pack(fill='x', padx=10, pady=2)
entry_tmr_freq = ttk.Entry(frame_freq, width=5); entry_tmr_freq.insert(0, "8")
entry_ch_freq = ttk.Entry(frame_freq, width=5); entry_ch_freq.insert(0, "1")
entry_freq = ttk.Entry(frame_freq, width=10); entry_freq.insert(0, "100000")
for w in (entry_tmr_freq, entry_ch_freq, entry_freq):
    w.pack(side='left', padx=2)
ttk.Button(frame_freq, text='发送Freq', command=send_pwm_freq).pack(side='left', padx=5)

# DAC 控制
ttk.Label(root, text="DAC 控制").pack(pady=5)
frame_dac = ttk.Frame(root)
frame_dac.pack(fill='x', padx=10, pady=2)
entry_dac_num = ttk.Entry(frame_dac, width=5); entry_dac_num.insert(0, "1")
entry_dac_val = ttk.Entry(frame_dac, width=10); entry_dac_val.insert(0, "3095")
for w in (entry_dac_num, entry_dac_val):
    w.pack(side='left', padx=2)
ttk.Button(frame_dac, text='发送DAC', command=send_dac).pack(side='left', padx=5)

# 其他命令
ttk.Label(root, text="其他命令").pack(pady=5)
frame_misc = ttk.Frame(root)
frame_misc.pack(fill='x', padx=10, pady=2)
ttk.Button(frame_misc, text='查询温度', command=query_temperature).pack(side='left', padx=5)
ttk.Button(frame_misc, text='查询状态', command=query_status).pack(side='left', padx=5)
ttk.Button(frame_misc, text='帮助示例', command=help_example).pack(side='left', padx=5)
ttk.Button(frame_misc, text='清空回显', command=clear_text).pack(side='left', padx=5)



# 串口接收/回显区域
ttk.Label(root, text="接收 / 回显").pack(pady=5)
text_area = tk.Text(root, height=15)
text_area.pack(fill='both', expand=True, padx=10, pady=2)

# 初始化
refresh_serial_ports()
read_serial()

root.mainloop()
