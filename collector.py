import serial
import serial.tools.list_ports
import time
import csv
import os
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors



from datetime import datetime







# ══════════════════════════════════════════



# NUTRISCAN AI - DATASET COLLECTOR



# Classes: RIPE | ROTTEN



# ══════════════════════════════════════════







ESP32_PORT      = "COM13"   # change to your port



BAUD_RATE       = 115200



DATASET_FILE    = "mango_dataset.csv"



EXPECTED_POINTS = 55







WAVELENGTHS = [



    410, 435, 460, 485, 510, 535,



    560, 585, 610, 645, 680, 705,



    730, 760, 810, 860, 900, 940



]







HEIGHT_LABELS = ["Top","UpMid","Center","LoMid","Bottom"]







ROTATION_LABELS = [



    "0°","34°","68°","102°","136°","170°",



    "136°","102°","68°","34°","0°"



]







# ══════════════════════════════════════════



# CONNECT



# ══════════════════════════════════════════



def connect():



    print("Finding ports...")



    for p in serial.tools.list_ports.comports():



        print(f"  {p.device} -> {p.description}")







    try:



        print(f"\nConnecting on {ESP32_PORT}...")



        ser = serial.Serial(ESP32_PORT, BAUD_RATE, timeout=60)



        time.sleep(2)







        start = time.time()



        while time.time() - start < 10:



            if ser.in_waiting:



                line = ser.readline().decode(



                    "utf-8", errors="ignore").strip()



                print(f"ESP32: {line}")



                if "READY" in line:



                    print("✅ Connected!")



                    return ser







        print("⚠ No READY — press RESET on ESP32")



        return ser







    except Exception as e:



        print(f"❌ Cannot connect: {e}")



        return None







# ══════════════════════════════════════════



# RUN SCAN



# ══════════════════════════════════════════



def run_scan(ser):



    print("\nSending SCAN command...")



    ser.reset_input_buffer()



    ser.write(b"SCAN\n")







    scan_data = []



    print("Do NOT touch anything...\n")







    while True:



        if ser.in_waiting > 0:



            try:



                line = ser.readline().decode(



                    "utf-8", errors="ignore").strip()



            except:



                continue







            if "SCANNING MANGO AT" in line:



                print(f"  {line}")







            if "SCAN_END" in line:



                print(f"\n✅ Scan done! "



                      f"{len(scan_data)} points")



                break







            if line.startswith("R:"):



                try:



                    parts = line.split(",")



                    rot  = int(parts[0].replace("R:","").strip())



                    vert = int(parts[1].replace("V:","").strip())



                    vals = [float(x) for x in parts[2:20]]







                    if len(vals) == 18:



                        pass_index = len(scan_data) // 5



                        if pass_index < 11:



                            nir = vals[17]



                            scan_data.append({



                                "rot":  rot,



                                "pass": pass_index,



                                "vert": vert,



                                "vals": vals



                            })



                            print(f"  Pass:{pass_index:02d} "



                                  f"R:{rot} V:{vert} "



                                  f"→ 940nm:{nir:.2f}")



                except:



                    pass



        else:



            time.sleep(0.05)







    return scan_data







# ══════════════════════════════════════════



# SHOW HEATMAP



# ══════════════════════════════════════════







def show_heatmap(scan_data, label):



    if not scan_data:



        print("No scan data available.")



        return 0.0, 0.0, 0.0, "UNKNOWN"







    n_rot, n_vert = 11, 5



    grid = np.full((n_vert, n_rot), np.nan)







    # Fill heatmap using the 940 nm readings



    for d in scan_data:



        p, v = d["pass"], d["vert"]



        if 0 <= p < n_rot and 0 <= v < n_vert:



            grid[v, p] = d["vals"][17]







    values = grid[~np.isnan(grid)]







    if len(values) == 0:



        print("No valid readings.")



        return 0.0, 0.0, 0.0, "UNKNOWN"







    nir_min = float(np.min(values))



    nir_max = float(np.max(values))



    nir_avg = float(np.mean(values))







    # Current working ranges for 940 nm: EMPTY < 10, ROTTEN 10-250, RIPE > 250



    empty_count = np.sum(values < 10)



    rotten_count = np.sum((values >= 10) & (values <= 250))



    ripe_count = np.sum(values > 250)







    total = len(values)







    empty_pct = empty_count / total * 100



    rotten_pct = rotten_count / total * 100



    ripe_pct = ripe_count / total * 100







    # Final decision:



    # At least 90% EMPTY readings means no mango detected.



    # Otherwise, compare RIPE and ROTTEN among non-empty readings.



    non_empty_count = ripe_count + rotten_count







    if empty_pct >= 90:



        verdict = "EMPTY"



        color = "#888888"



    elif non_empty_count == 0:



        verdict = "EMPTY"



        color = "#888888"



    elif ripe_count > rotten_count:



        verdict = "RIPE"



        color = "#00cc66"



    elif rotten_count > ripe_count:



        verdict = "ROTTEN"



        color = "#ff4444"



    else:



        verdict = "MIXED - TIE"



        color = "#ffaa00"







    # Print percentages



    print("\n========== MANGO SCAN REPORT ==========")



    print(f"940 nm minimum : {nir_min:.2f}")



    print(f"940 nm maximum : {nir_max:.2f}")



    print(f"940 nm average : {nir_avg:.2f}")



    print(f"Valid points   : {total}")



    print(f"EMPTY          : {empty_pct:.1f}%")



    print(f"ROTTEN         : {rotten_pct:.1f}%")



    print(f"RIPE           : {ripe_pct:.1f}%")



    print(f"FINAL RESULT   : {verdict}")



    print("=======================================\n")







    # Create heatmap



    fig, axes = plt.subplots(1, 2, figsize=(16, 5))



    fig.patch.set_facecolor("#0a0e1a")







    cmap = mcolors.LinearSegmentedColormap.from_list(



        "nir",



        ["#440000", "#ff4400", "#ffff00", "#00cc44"]



    )



    cmap.set_bad("#111827")







    ax1 = axes[0]



    im = ax1.imshow(



        grid,



        cmap=cmap,



        vmin=0,



        vmax=max(500, nir_max),



        aspect="auto"



    )







    ax1.set_title(



        "940 nm Heatmap\n(5 heights x 11 passes)",



        color="#00f5ff"



    )



    ax1.set_xlabel("Rotation Pass", color="#8aaac8")



    ax1.set_ylabel("Height", color="#8aaac8")



    ax1.set_xticks(range(n_rot))



    ax1.set_xticklabels(



        ROTATION_LABELS,



        rotation=45,



        ha="right",



        color="#8aaac8",



        fontsize=8



    )



    ax1.set_yticks(range(n_vert))



    ax1.set_yticklabels(HEIGHT_LABELS, color="#8aaac8")



    ax1.set_facecolor("#050810")



    plt.colorbar(im, ax=ax1)







    for r in range(n_vert):



        for c in range(n_rot):



            v = grid[r, c]



            if not np.isnan(v):



                ax1.text(



                    c, r, f"{v:.0f}",



                    ha="center",



                    va="center",



                    fontsize=7,



                    color="white"



                )







    # Result and percentage panel



    ax2 = axes[1]



    ax2.set_facecolor("#050810")



    ax2.axis("off")







    ax2.text(



        0.5, 0.90, "MANGO SCAN REPORT",



        ha="center", fontsize=14,



        color="#00f5ff",



        transform=ax2.transAxes



    )







    ax2.text(



        0.5, 0.75, verdict,



        ha="center", fontsize=26,



        fontweight="bold", color=color,



        transform=ax2.transAxes



    )







    ax2.text(



        0.5, 0.59, f"RIPE: {ripe_pct:.1f}%",



        ha="center", fontsize=16,



        color="#00cc66",



        transform=ax2.transAxes



    )







    ax2.text(



        0.5, 0.47, f"ROTTEN: {rotten_pct:.1f}%",



        ha="center", fontsize=16,



        color="#ff4444",



        transform=ax2.transAxes



    )







    ax2.text(



        0.5, 0.35, f"EMPTY: {empty_pct:.1f}%",



        ha="center", fontsize=16,



        color="#aaaaaa",



        transform=ax2.transAxes



    )







    ax2.text(



        0.5, 0.23, f"Valid scan points: {total}/55",



        ha="center", fontsize=11,



        color="#8aaac8",



        transform=ax2.transAxes



    )







    ax2.text(



        0.5, 0.12, f"940 nm average: {nir_avg:.2f}",



        ha="center", fontsize=11,



        color="#8aaac8",



        transform=ax2.transAxes



    )







    fig.suptitle(



        f"{label} | {verdict}",



        fontsize=14,



        color=color,



        fontweight="bold"



    )







    plt.tight_layout()







    ts = datetime.now().strftime("%Y%m%d_%H%M%S")



    img = f"scan_{label}_{ts}.png"







    plt.savefig(



        img,



        dpi=120,



        facecolor="#0a0e1a",



        bbox_inches="tight"



    )







    print(f"Heatmap saved: {img}")



    plt.show()







    return ripe_pct, rotten_pct, empty_pct, verdict





def save_to_csv(scan_data, label, mango_id):



    file_exists = os.path.exists(DATASET_FILE)







    with open(DATASET_FILE, "a", newline="",



              encoding="utf-8") as f:



        writer = csv.writer(f)







        if not file_exists:



            header = (



                ["label","mango_id",



                 "rotation_pass","height_pos"]



                + [f"nm{w}" for w in WAVELENGTHS]



            )



            writer.writerow(header)







        for d in scan_data:



            row = (



                [label, mango_id,



                 d["pass"], d["vert"]]



                + [f"{v:.4f}" for v in d["vals"]]



            )



            writer.writerow(row)







    print(f"✅ {len(scan_data)} rows → {DATASET_FILE}")







# ══════════════════════════════════════════



# MAIN



# ══════════════════════════════════════════



def main():



    print()



    print("=" * 50)



    print("    NUTRISCAN AI — DATASET COLLECTOR")



    print("    Classes: RIPE | ROTTEN")



    print("=" * 50)



    print()







    ser = connect()



    if ser is None:



        input("Press Enter to exit...")



        return







    count = {"RIPE": 0, "ROTTEN": 0}







    try:



        while True:



            print()



            print("─" * 50)



            print(f"Collected: "



                  f"RIPE={count['RIPE']}  "



                  f"ROTTEN={count['ROTTEN']}")



            print()



            print("Type: RIPE or ROTTEN or QUIT")



            label = input("Your choice: ").strip().upper()







            if label == "QUIT":



                break







            if label not in ["RIPE", "ROTTEN"]:



                print("❌ Type RIPE or ROTTEN only")



                continue







            count[label] += 1



            mango_id = f"{label[:2]}{count[label]}"







            print(f"\nSample ID: {mango_id}")



            print("─" * 50)



            print("1. Place mango vertically on turntable")



            print("2. Stem pointing UP")



            print("3. Center on disc")



            print("4. Close box cover")



            print("─" * 50)



            input("Press Enter when ready...")







            scan_data = run_scan(ser)







            # Incomplete scan check



            if len(scan_data) != EXPECTED_POINTS:



                print(f"\n❌ Incomplete scan!")



                print(f"   Expected: {EXPECTED_POINTS}")



                print(f"   Got:      {len(scan_data)}")



                print("   NOT saved. Try again.")



                count[label] -= 1



                continue

            ripe_pct, rotten_pct, empty_pct, verdict = show_heatmap(

                scan_data, label

            )

            save_to_csv(scan_data, label, mango_id)







            print()



            print("═" * 50)



            print(f"  SAMPLE  : {mango_id}")

            print(f"  INPUT LABEL : {label}")

            print(f"  RIPE        : {ripe_pct:.1f}%")

            print(f"  ROTTEN      : {rotten_pct:.1f}%")

            print(f"  EMPTY       : {empty_pct:.1f}%")

            print(f"  FINAL RESULT: {verdict}")

            print("═" * 50)







    except KeyboardInterrupt:



        print("\nStopped.")



    finally:



        ser.close()



        total = sum(count.values())



        print()



        print("── FINAL SUMMARY ──")



        print(f"RIPE   : {count['RIPE']} mangoes")



        print(f"ROTTEN : {count['ROTTEN']} mangoes")



        print(f"TOTAL  : {total} mangoes")



        print(f"FILE   : {DATASET_FILE}")



        input("\nPress Enter to close...")







if __name__ == "__main__":



    main()