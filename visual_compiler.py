# -*- coding: utf-8 -*-
"""
Mini-ProLang Studio
Interface visuelle pour compiler et visualiser un projet ProLang.

Place ce fichier dans le même dossier que :
lexical.l, syn.y, quad.c, quad.h, optim.c, optim.h, codegen.c, codegen.h, ts.h

Commandes utilisées :
1) bison -d syn.y
2) flex lexical.l
3) gcc lex.yy.c syn.tab.c quad.c optim.c codegen.c -o prolang.exe
4) prolang.exe reçoit le programme ProLang depuis l'éditeur

Compatible Windows / Linux, mais pensé pour Windows + MinGW/Git Bash + EMU8086.
"""

import os
import sys
import time
import shutil
import subprocess
import threading
import tkinter as tk
from tkinter import ttk, filedialog, messagebox
from pathlib import Path


DEFAULT_SOURCE = """BeginProject Projetm1;
Setup:
//*
    DECLARATIONS
*//

%% variables simples
define x | y | z : integer;
define i | j | k : integer;
define a | b : float;

%% avec initialisation
define somme : integer = 0;
define moyenne : float = 0.0;

%% tableaux
define Tabint : [integer; 50];
define Tabfloat : [float; 30];

%% constantes
const Pi : float = 3.14159;
const Max : integer = 100;

Run:
{
    x <-- 10;
    y <-- 5;
    z <-- 2;

    a <-- 2.5;
    b <-- (a + Pi) * 2.0;

    Tabint[0] <-- x + y * z;
    Tabfloat[1] <-- (b + 3.5) / 2.0;

    if ((x > y AND (z < x + y)) OR NON(y == 0)) then:
    {
        somme <-- x + y + z;
    } else {
        somme <-- 0;
    } endIf;

    input(x);
    out("x: ", x);
    out(" Somme: ", somme);
    out(" Moyenne: ", moyenne);
}
EndProject;
"""


class ProLangStudio(tk.Tk):
    def __init__(self):
        super().__init__()

        self.title("Mini-ProLang Studio - Interface visuelle de compilation")
        self.geometry("1280x760")
        self.minsize(1050, 650)

        self.base_dir = Path(__file__).resolve().parent
        self.source_file = self.base_dir / "test.txt"
        self.output_asm = self.base_dir / "output.asm"
        self.exe_name = "prolang.exe" if os.name == "nt" else "prolang"
        self.exe_path = self.base_dir / self.exe_name

        self.bison_cmd = tk.StringVar(value="bison")
        self.flex_cmd = tk.StringVar(value="flex")
        self.gcc_cmd = tk.StringVar(value="gcc")
        self.emu_path = tk.StringVar(value="")
        self.status = tk.StringVar(value="Prêt")

        self._highlight_job = None
        self._build_ui()
        self._load_initial_source()
        self._apply_tags()

    # ------------------------------------------------------------------
    # UI
    # ------------------------------------------------------------------
    def _build_ui(self):
        self._make_style()

        top = ttk.Frame(self, padding=(10, 8))
        top.pack(side=tk.TOP, fill=tk.X)

        title = ttk.Label(top, text="Mini-ProLang Studio", style="Title.TLabel")
        title.pack(side=tk.LEFT, padx=(0, 20))

        ttk.Button(top, text="Nouveau", command=self.new_file).pack(side=tk.LEFT, padx=3)
        ttk.Button(top, text="Ouvrir", command=self.open_source).pack(side=tk.LEFT, padx=3)
        ttk.Button(top, text="Sauvegarder", command=self.save_source).pack(side=tk.LEFT, padx=3)
        ttk.Separator(top, orient=tk.VERTICAL).pack(side=tk.LEFT, fill=tk.Y, padx=10)
        ttk.Button(top, text="1. Construire compilateur", command=self.build_compiler_async).pack(side=tk.LEFT, padx=3)
        ttk.Button(top, text="2. Compiler le code", command=self.run_compiler_async).pack(side=tk.LEFT, padx=3)
        ttk.Button(top, text="Tout lancer", command=self.build_and_run_async).pack(side=tk.LEFT, padx=3)
        ttk.Separator(top, orient=tk.VERTICAL).pack(side=tk.LEFT, fill=tk.Y, padx=10)
        ttk.Button(top, text="Ouvrir output.asm", command=self.open_asm_external).pack(side=tk.LEFT, padx=3)
        ttk.Button(top, text="Lancer EMU8086", command=self.open_in_emulator).pack(side=tk.LEFT, padx=3)

        main = ttk.PanedWindow(self, orient=tk.HORIZONTAL)
        main.pack(side=tk.TOP, fill=tk.BOTH, expand=True, padx=10, pady=(0, 8))

        left = ttk.Frame(main)
        right = ttk.Frame(main)
        main.add(left, weight=3)
        main.add(right, weight=2)

        # Editor with line numbers
        editor_frame = ttk.Frame(left)
        editor_frame.pack(fill=tk.BOTH, expand=True)
        ttk.Label(editor_frame, text="Éditeur ProLang", style="Section.TLabel").pack(anchor="w")

        editor_inner = ttk.Frame(editor_frame)
        editor_inner.pack(fill=tk.BOTH, expand=True)

        self.lines = tk.Text(
            editor_inner,
            width=5,
            padx=4,
            takefocus=0,
            border=0,
            background="#f3f4f6",
            foreground="#6b7280",
            state="disabled",
            font=("Consolas", 11),
        )
        self.lines.pack(side=tk.LEFT, fill=tk.Y)

        self.editor = tk.Text(
            editor_inner,
            undo=True,
            wrap="none",
            font=("Consolas", 11),
            background="#ffffff",
            foreground="#111827",
            insertbackground="#0b3b75",
        )
        self.editor.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        yscroll = ttk.Scrollbar(editor_inner, orient=tk.VERTICAL, command=self._on_scroll)
        yscroll.pack(side=tk.RIGHT, fill=tk.Y)
        self.editor.configure(yscrollcommand=lambda *args: self._sync_scroll(args, yscroll))

        xscroll = ttk.Scrollbar(editor_frame, orient=tk.HORIZONTAL, command=self.editor.xview)
        xscroll.pack(fill=tk.X)
        self.editor.configure(xscrollcommand=xscroll.set)

        self.editor.bind("<KeyRelease>", self._on_editor_change)
        self.editor.bind("<MouseWheel>", lambda e: self.after(10, self._update_line_numbers))
        self.editor.bind("<ButtonRelease-1>", lambda e: self.after(10, self._update_line_numbers))

        # Right notebook
        self.tabs = ttk.Notebook(right)
        self.tabs.pack(fill=tk.BOTH, expand=True)

        self.console = self._make_text_tab("Console")
        self.asm_view = self._make_text_tab("output.asm")
        self.report_view = self._make_text_tab("Rapport")
        self.help_view = self._make_text_tab("Aide")

        self._write_help()

        bottom = ttk.Frame(self, padding=(10, 0, 10, 8))
        bottom.pack(side=tk.BOTTOM, fill=tk.X)

        ttk.Label(bottom, text="Dossier projet :").pack(side=tk.LEFT)
        self.project_label = ttk.Label(bottom, text=str(self.base_dir), style="Path.TLabel")
        self.project_label.pack(side=tk.LEFT, padx=(5, 12))
        ttk.Button(bottom, text="Changer dossier", command=self.change_project_dir).pack(side=tk.LEFT, padx=3)
        ttk.Button(bottom, text="Paramètres", command=self.open_settings).pack(side=tk.LEFT, padx=3)

        status_bar = ttk.Label(self, textvariable=self.status, anchor="w", padding=(10, 4), style="Status.TLabel")
        status_bar.pack(side=tk.BOTTOM, fill=tk.X)

    def _make_style(self):
        style = ttk.Style(self)
        if "clam" in style.theme_names():
            style.theme_use("clam")
        style.configure("Title.TLabel", font=("Segoe UI", 15, "bold"), foreground="#0b3b75")
        style.configure("Section.TLabel", font=("Segoe UI", 11, "bold"), foreground="#0b3b75", padding=(0, 0, 0, 4))
        style.configure("Path.TLabel", foreground="#374151")
        style.configure("Status.TLabel", foreground="#ffffff", background="#0b3b75")
        style.configure("TButton", padding=(8, 4))

    def _make_text_tab(self, name):
        frame = ttk.Frame(self.tabs)
        self.tabs.add(frame, text=name)
        text = tk.Text(
            frame,
            wrap="none",
            font=("Consolas", 10),
            background="#0f172a" if name == "Console" else "#ffffff",
            foreground="#e5e7eb" if name == "Console" else "#111827",
            insertbackground="#ffffff" if name == "Console" else "#111827",
        )
        text.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        y = ttk.Scrollbar(frame, orient=tk.VERTICAL, command=text.yview)
        y.pack(side=tk.RIGHT, fill=tk.Y)
        x = ttk.Scrollbar(frame, orient=tk.HORIZONTAL, command=text.xview)
        x.pack(side=tk.BOTTOM, fill=tk.X)
        text.configure(yscrollcommand=y.set, xscrollcommand=x.set)
        return text

    # ------------------------------------------------------------------
    # Editor helpers
    # ------------------------------------------------------------------
    def _load_initial_source(self):
        if self.source_file.exists():
            content = self.source_file.read_text(encoding="utf-8", errors="replace")
        else:
            content = DEFAULT_SOURCE
        self.editor.delete("1.0", tk.END)
        self.editor.insert("1.0", content)
        self._update_line_numbers()
        self._highlight_now()

    def _on_scroll(self, *args):
        self.editor.yview(*args)
        self.lines.yview(*args)

    def _sync_scroll(self, args, scrollbar):
        scrollbar.set(*args)
        self.lines.yview_moveto(args[0])
        self._update_line_numbers()

    def _update_line_numbers(self):
        self.lines.configure(state="normal")
        self.lines.delete("1.0", tk.END)
        line_count = int(self.editor.index("end-1c").split(".")[0])
        self.lines.insert("1.0", "\n".join(str(i) for i in range(1, line_count + 1)))
        self.lines.configure(state="disabled")

    def _on_editor_change(self, event=None):
        self._update_line_numbers()
        if self._highlight_job:
            self.after_cancel(self._highlight_job)
        self._highlight_job = self.after(250, self._highlight_now)

    def _apply_tags(self):
        self.editor.tag_configure("kw", foreground="#0b3b75", font=("Consolas", 11, "bold"))
        self.editor.tag_configure("type", foreground="#7c3aed", font=("Consolas", 11, "bold"))
        self.editor.tag_configure("str", foreground="#047857")
        self.editor.tag_configure("num", foreground="#b45309")
        self.editor.tag_configure("comment", foreground="#6b7280", font=("Consolas", 11, "italic"))
        self.editor.tag_configure("op", foreground="#dc2626", font=("Consolas", 11, "bold"))

    def _highlight_now(self):
        import re

        text = self.editor.get("1.0", "end-1c")
        for tag in ("kw", "type", "str", "num", "comment", "op"):
            self.editor.tag_remove(tag, "1.0", tk.END)

        patterns = [
            ("comment", r"//\*.*?\*//"),
            ("comment", r"%%.*?$"),
            ("str", r'"[^"\\]*(?:\\.[^"\\]*)*"'),
            ("kw", r"\b(BeginProject|Setup|Run|EndProject|define|const|if|then|else|endIf|loop|while|endloop|for|in|to|endfor|input|out|AND|OR|NON)\b"),
            ("type", r"\b(integer|float)\b"),
            ("num", r"\b[0-9]+(?:\.[0-9]+)?\b"),
            ("op", r"<--|>=|<=|==|!=|[+\-*/<>:;{}\[\]()]"),
        ]

        for tag, pat in patterns:
            flags = re.DOTALL | re.MULTILINE if tag == "comment" else re.MULTILINE
            for m in re.finditer(pat, text, flags):
                start = f"1.0+{m.start()}c"
                end = f"1.0+{m.end()}c"
                self.editor.tag_add(tag, start, end)

    # ------------------------------------------------------------------
    # File operations
    # ------------------------------------------------------------------
    def new_file(self):
        if messagebox.askyesno("Nouveau", "Effacer le contenu actuel ?"):
            self.editor.delete("1.0", tk.END)
            self.editor.insert("1.0", DEFAULT_SOURCE)
            self._on_editor_change()

    def open_source(self):
        path = filedialog.askopenfilename(
            title="Ouvrir un programme ProLang",
            filetypes=[("Text/ProLang", "*.txt *.plang *.pro"), ("Tous les fichiers", "*.*")],
            initialdir=self.base_dir,
        )
        if not path:
            return
        p = Path(path)
        self.editor.delete("1.0", tk.END)
        self.editor.insert("1.0", p.read_text(encoding="utf-8", errors="replace"))
        self.source_file = p
        self._on_editor_change()
        self.status.set(f"Fichier ouvert : {p.name}")

    def save_source(self):
        content = self.editor.get("1.0", "end-1c")
        self.source_file.write_text(content, encoding="utf-8")
        self.status.set(f"Programme sauvegardé : {self.source_file}")
        return self.source_file

    def change_project_dir(self):
        path = filedialog.askdirectory(title="Choisir le dossier du projet", initialdir=self.base_dir)
        if not path:
            return
        self.base_dir = Path(path)
        self.source_file = self.base_dir / "test.txt"
        self.output_asm = self.base_dir / "output.asm"
        self.exe_path = self.base_dir / self.exe_name
        self.project_label.config(text=str(self.base_dir))
        self._load_initial_source()
        self.clear_console()
        self.log(f"Dossier projet changé : {self.base_dir}\n")

    # ------------------------------------------------------------------
    # Settings
    # ------------------------------------------------------------------
    def open_settings(self):
        win = tk.Toplevel(self)
        win.title("Paramètres")
        win.geometry("720x260")
        win.transient(self)
        win.grab_set()

        frm = ttk.Frame(win, padding=12)
        frm.pack(fill=tk.BOTH, expand=True)

        rows = [
            ("Commande Bison", self.bison_cmd),
            ("Commande Flex", self.flex_cmd),
            ("Commande GCC", self.gcc_cmd),
            ("Chemin EMU8086.exe", self.emu_path),
        ]

        for r, (label, var) in enumerate(rows):
            ttk.Label(frm, text=label).grid(row=r, column=0, sticky="w", pady=6)
            entry = ttk.Entry(frm, textvariable=var, width=70)
            entry.grid(row=r, column=1, sticky="we", padx=8, pady=6)
            if label.startswith("Chemin"):
                ttk.Button(frm, text="Parcourir", command=lambda: self._choose_emu()).grid(row=r, column=2, padx=4)

        frm.columnconfigure(1, weight=1)

        info = (
            "Exemple Windows : C:\\emu8086\\emu8086.exe\n"
            "Si EMU8086 n'est pas configuré, le bouton ouvrira simplement output.asm avec le programme associé."
        )
        ttk.Label(frm, text=info, foreground="#4b5563").grid(row=5, column=0, columnspan=3, sticky="w", pady=(12, 0))
        ttk.Button(frm, text="Fermer", command=win.destroy).grid(row=6, column=2, sticky="e", pady=12)

    def _choose_emu(self):
        path = filedialog.askopenfilename(
            title="Choisir emu8086.exe",
            filetypes=[("Executable", "*.exe"), ("Tous les fichiers", "*.*")],
        )
        if path:
            self.emu_path.set(path)

    # ------------------------------------------------------------------
    # Console helpers
    # ------------------------------------------------------------------
    def clear_console(self):
        self.console.delete("1.0", tk.END)

    def log(self, msg):
        self.console.insert(tk.END, msg)
        self.console.see(tk.END)
        self.update_idletasks()

    def set_text(self, widget, content):
        widget.delete("1.0", tk.END)
        widget.insert("1.0", content)

    def _write_help(self):
        help_text = """
Mini-ProLang Studio
===================

Objectif :
- Écrire un programme ProLang dans l'éditeur.
- Construire le compilateur Flex/Bison/GCC.
- Compiler le programme ProLang.
- Voir la console, le code assembleur output.asm et le rapport.
- Ouvrir output.asm dans EMU8086.

Étapes conseillées :
1) Mets visual_compiler.py dans le même dossier que lexical.l, syn.y, quad.c, optim.c, codegen.c, ts.h.
2) Clique sur "Tout lancer".
3) Vérifie l'onglet Console.
4) Vérifie l'onglet output.asm.
5) Clique sur "Lancer EMU8086".

Commandes utilisées :
- bison -d syn.y
- flex lexical.l
- gcc lex.yy.c syn.tab.c quad.c optim.c codegen.c -o prolang.exe
- prolang.exe reçoit le programme ProLang depuis l'éditeur.

Remarque :
Si le compilateur demande input(x), dans EMU8086 tu tapes une valeur entière simple, par exemple 7.
""".strip()
        self.set_text(self.help_view, help_text)

    # ------------------------------------------------------------------
    # Compilation commands
    # ------------------------------------------------------------------
    def run_cmd(self, cmd, input_text=None, timeout=30):
        self.log(f"\n$ {' '.join(map(str, cmd))}\n")
        try:
            result = subprocess.run(
                cmd,
                cwd=str(self.base_dir),
                input=input_text,
                text=True,
                capture_output=True,
                timeout=timeout,
                shell=False,
            )
        except FileNotFoundError as e:
            return 127, "", f"Commande introuvable : {cmd[0]}\nDétail : {e}\n"
        except subprocess.TimeoutExpired:
            return 124, "", f"Timeout : la commande a dépassé {timeout} secondes.\n"
        except Exception as e:
            return 1, "", f"Erreur inattendue : {e}\n"

        if result.stdout:
            self.log(result.stdout)
        if result.stderr:
            self.log(result.stderr)
        self.log(f"[code retour = {result.returncode}]\n")
        return result.returncode, result.stdout, result.stderr

    def build_compiler_async(self):
        threading.Thread(target=self.build_compiler, daemon=True).start()

    def run_compiler_async(self):
        threading.Thread(target=self.run_compiler_on_source, daemon=True).start()

    def build_and_run_async(self):
        threading.Thread(target=self.build_and_run, daemon=True).start()

    def build_compiler(self):
        self.status.set("Construction du compilateur...")
        self.clear_console()
        self.log("=== Construction du compilateur Flex/Bison/GCC ===\n")
        self._check_project_files()

        commands = [
            [self.bison_cmd.get(), "-d", "syn.y"],
            [self.flex_cmd.get(), "lexical.l"],
            [self.gcc_cmd.get(), "lex.yy.c", "syn.tab.c", "quad.c", "optim.c", "codegen.c", "-o", self.exe_name],
        ]

        for cmd in commands:
            code, _, _ = self.run_cmd(cmd, timeout=60)
            if code != 0:
                self.status.set("Erreur pendant la construction")
                messagebox.showerror("Erreur", "La construction du compilateur a échoué. Voir l'onglet Console.")
                return False

        self.status.set("Compilateur construit avec succès")
        self.log("\n✅ Compilateur construit avec succès.\n")
        return True

    def run_compiler_on_source(self):
        self.status.set("Compilation du programme ProLang...")
        self.save_source()
        source = self.editor.get("1.0", "end-1c")

        if not self.exe_path.exists():
            self.log("\n⚠ prolang.exe introuvable. Lance d'abord la construction.\n")
            self.status.set("prolang.exe introuvable")
            return False

        self.log("\n=== Compilation du programme source ProLang ===\n")
        exe_cmd = [str(self.exe_path)]
        code, out, err = self.run_cmd(exe_cmd, input_text=source, timeout=30)

        self.refresh_asm_view()
        self.generate_visual_report(out, err, code)

        if code != 0:
            self.status.set("Compilation terminée avec erreurs")
            return False

        self.status.set("Compilation ProLang terminée")
        return True

    def build_and_run(self):
        ok = self.build_compiler()
        if ok:
            self.run_compiler_on_source()

    def _check_project_files(self):
        required = ["lexical.l", "syn.y", "quad.c", "optim.c", "codegen.c", "quad.h", "optim.h", "codegen.h", "ts.h"]
        missing = [f for f in required if not (self.base_dir / f).exists()]
        if missing:
            self.log("\n⚠ Fichiers manquants :\n")
            for f in missing:
                self.log(f"  - {f}\n")
            self.log("Le build peut échouer si ces fichiers sont nécessaires.\n")

    # ------------------------------------------------------------------
    # Visual reports
    # ------------------------------------------------------------------
    def refresh_asm_view(self):
        asm_path = self.find_asm_file()
        if asm_path and asm_path.exists():
            content = asm_path.read_text(encoding="utf-8", errors="replace")
            self.set_text(self.asm_view, content)
            self.output_asm = asm_path
        else:
            self.set_text(self.asm_view, "Aucun fichier .asm trouvé.")

    def find_asm_file(self):
        preferred = [self.base_dir / "output.asm", self.base_dir / "outputResultat.asm", self.base_dir / "out.asm"]
        for p in preferred:
            if p.exists():
                return p
        asm_files = list(self.base_dir.glob("*.asm"))
        if not asm_files:
            return None
        asm_files.sort(key=lambda p: p.stat().st_mtime, reverse=True)
        return asm_files[0]

    def generate_visual_report(self, stdout, stderr, returncode):
        asm_path = self.find_asm_file()
        asm_status = "Oui" if asm_path else "Non"
        asm_name = asm_path.name if asm_path else "-"

        report = []
        report.append("VISUALISATION DE LA COMPILATION")
        report.append("================================")
        report.append("")
        report.append(f"Dossier projet        : {self.base_dir}")
        report.append(f"Fichier source        : {self.source_file.name}")
        report.append(f"Code retour           : {returncode}")
        report.append(f"Fichier ASM généré    : {asm_status} ({asm_name})")
        report.append("")
        report.append("Flux global :")
        report.append("Source ProLang -> Flex -> Bison -> Table des symboles -> Quadruplets -> Optimisation -> output.asm -> EMU8086")
        report.append("")
        report.append("Résumé console :")
        report.append("----------------")
        combined = (stdout or "") + ("\n" + stderr if stderr else "")
        if combined.strip():
            report.append(combined.strip())
        else:
            report.append("Aucune sortie console.")
        report.append("")
        report.append("Extrait output.asm :")
        report.append("-------------------")
        if asm_path and asm_path.exists():
            asm_lines = asm_path.read_text(encoding="utf-8", errors="replace").splitlines()
            report.extend(asm_lines[:80])
            if len(asm_lines) > 80:
                report.append("...")
        else:
            report.append("Aucun fichier assembleur trouvé.")

        content = "\n".join(report)
        self.set_text(self.report_view, content)
        try:
            (self.base_dir / "rapport_visualisation.txt").write_text(content, encoding="utf-8")
        except Exception:
            pass

    # ------------------------------------------------------------------
    # External opening
    # ------------------------------------------------------------------
    def open_asm_external(self):
        asm_path = self.find_asm_file()
        if not asm_path:
            messagebox.showwarning("output.asm", "Aucun fichier .asm trouvé. Compile d'abord le programme.")
            return
        self._open_file_default(asm_path)

    def open_in_emulator(self):
        asm_path = self.find_asm_file()
        if not asm_path:
            messagebox.showwarning("EMU8086", "Aucun fichier .asm trouvé. Compile d'abord le programme.")
            return

        emu = self.emu_path.get().strip()
        try:
            if emu and Path(emu).exists():
                subprocess.Popen([emu, str(asm_path)], cwd=str(self.base_dir))
                self.status.set("Ouverture dans EMU8086...")
            else:
                self._open_file_default(asm_path)
                self.status.set("output.asm ouvert avec le programme associé")
        except Exception as e:
            messagebox.showerror("Erreur EMU8086", str(e))

    def _open_file_default(self, path):
        path = str(path)
        if os.name == "nt":
            os.startfile(path)  # type: ignore[attr-defined]
        elif sys.platform == "darwin":
            subprocess.Popen(["open", path])
        else:
            subprocess.Popen(["xdg-open", path])


def main():
    app = ProLangStudio()
    app.mainloop()


if __name__ == "__main__":
    main()
