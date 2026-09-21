using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Globalization;
using System.IO;
using System.Net;
using System.Security.Cryptography;
using System.Text;
using System.Threading;
using System.Windows.Forms;

namespace NlcLauncher
{
    static class Cfg
    {
        public static string GameRoot;
        public static string Repo = "jordan1227/improved_to_he";
        public static string Ref = "main";
        public static string ManifestName = "manifest.txt";
        public static string Base = "";    // override download root (mirror / test server)
        public static string GameExe = @"bin_x64\xrEngine.exe";
        public static bool NoUpdate;      // -noupdate : launch at once
        public static bool NoLaunch;      // -nolaunch : update only
        public static bool ExitWhenDone;  // -exit     : close instead of waiting
        public static bool AssumeYes;     // -yes      : do not ask before touching foreign files
        public static bool DevOk;         // -devok    : update even in a development checkout
        public static bool NoKeep;        // -nokeep   : sync even the files an optional variant owns
        public static string ExtraArgs = "";

        public static string StateDir { get { return Path.Combine(GameRoot, @"appdata\updater"); } }
        public static string StateFile { get { return Path.Combine(StateDir, "state.txt"); } }
        public static string TempDir { get { return Path.Combine(StateDir, "tmp"); } }
        public static string LogFile { get { return Path.Combine(StateDir, "updater.log"); } }
        public static string BackupRoot { get { return Path.Combine(StateDir, "backup"); } }

        public static void LoadIni()
        {
            string ini = Path.Combine(GameRoot, "updater.ini");
            if (!File.Exists(ini)) return;
            foreach (string raw in File.ReadAllLines(ini, Encoding.UTF8))
            {
                string line = raw.Trim();
                if (line.Length == 0 || line[0] == ';' || line[0] == '#') continue;
                int eq = line.IndexOf('=');
                if (eq <= 0) continue;
                string k = line.Substring(0, eq).Trim().ToLowerInvariant();
                string v = line.Substring(eq + 1).Trim();
                if (k == "repo") Repo = v;
                else if (k == "ref") Ref = v;
                else if (k == "manifest") ManifestName = v;
                else if (k == "base") Base = v.TrimEnd('/');
                else if (k == "game") GameExe = v;
                else if (k == "devok") DevOk = (v == "1" || v.ToLowerInvariant() == "true");
                else if (k == "nokeep") NoKeep = (v == "1" || v.ToLowerInvariant() == "true");
            }
        }

        public static void ParseArgs(string[] args)
        {
            var rest = new List<string>();
            foreach (string a in args)
            {
                string low = a.ToLowerInvariant();
                if (low == "-noupdate" || low == "/noupdate") NoUpdate = true;
                else if (low == "-nolaunch" || low == "/nolaunch") NoLaunch = true;
                else if (low == "-exit" || low == "/exit") ExitWhenDone = true;
                else if (low == "-yes" || low == "/yes") AssumeYes = true;
                else if (low == "-devok" || low == "/devok") DevOk = true;
                else if (low == "-nokeep" || low == "/nokeep") NoKeep = true;
                else rest.Add(a);
            }
            ExtraArgs = string.Join(" ", rest.ToArray());
        }
    }

    class Entry
    {
        public string Path;
        public string Sha;
        public long Size;
        public bool LocalExists;   // a file is already there
        public bool Ours;          // ...and the updater is the one that put it there
    }

    class StateRec
    {
        public string Sha;
        public long Size;
        public long Mtime;
    }

    static class Manifest
    {
        public static string Version = "";

        // paths an optional variant owns, per variant name: the build ships its own
        // versions of them, and installing those on top of the variant undoes it.
        public static Dictionary<string, HashSet<string>> Keep =
            new Dictionary<string, HashSet<string>>(StringComparer.OrdinalIgnoreCase);

        public static Dictionary<string, Entry> Parse(string text)
        {
            var map = new Dictionary<string, Entry>(StringComparer.OrdinalIgnoreCase);
            Keep = new Dictionary<string, HashSet<string>>(StringComparer.OrdinalIgnoreCase);
            foreach (string raw in text.Split('\n'))
            {
                string line = raw.TrimEnd('\r');
                if (line.Length == 0) continue;
                if (line[0] == '#')
                {
                    if (line.StartsWith("#version ")) Version = line.Substring(9).Trim();
                    else if (line.StartsWith("#keep ")) ParseKeep(line.Substring(6));
                    continue;
                }
                int a = line.IndexOf(' ');
                if (a < 0) continue;
                int b = line.IndexOf(' ', a + 1);
                if (b < 0) continue;
                var e = new Entry();
                e.Sha = line.Substring(0, a);
                e.Size = long.Parse(line.Substring(a + 1, b - a - 1), CultureInfo.InvariantCulture);
                e.Path = line.Substring(b + 1).Replace('\\', '/').Trim();
                if (e.Path.Length == 0 || e.Sha.Length != 64) continue;
                map[e.Path] = e;
            }
            return map;
        }

        static void ParseKeep(string rest)
        {
            rest = rest.Trim();
            int sp = rest.IndexOf(' ');
            if (sp <= 0) return;
            string name = rest.Substring(0, sp).Trim();
            string path = rest.Substring(sp + 1).Replace('\\', '/').Trim();
            if (name.Length == 0 || path.Length == 0) return;
            HashSet<string> set;
            if (!Keep.TryGetValue(name, out set))
            {
                set = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
                Keep[name] = set;
            }
            set.Add(path);
        }
    }

    static class State
    {
        public static Dictionary<string, StateRec> Load()
        {
            var map = new Dictionary<string, StateRec>(StringComparer.OrdinalIgnoreCase);
            if (!File.Exists(Cfg.StateFile)) return map;
            try
            {
                foreach (string raw in File.ReadAllLines(Cfg.StateFile, Encoding.UTF8))
                {
                    string line = raw.Trim();
                    if (line.Length == 0 || line[0] == '#') continue;
                    string[] p = line.Split(new char[] { ' ' }, 4);
                    if (p.Length != 4) continue;
                    var r = new StateRec();
                    r.Sha = p[0];
                    r.Size = long.Parse(p[1], CultureInfo.InvariantCulture);
                    r.Mtime = long.Parse(p[2], CultureInfo.InvariantCulture);
                    map[p[3]] = r;
                }
            }
            catch { }
            return map;
        }

        public static void Save(Dictionary<string, StateRec> map)
        {
            try
            {
                Directory.CreateDirectory(Cfg.StateDir);
                var sb = new StringBuilder();
                sb.Append("#nlc-updater-state 1\n");
                foreach (var kv in map)
                    sb.AppendFormat(CultureInfo.InvariantCulture, "{0} {1} {2} {3}\n",
                        kv.Value.Sha, kv.Value.Size, kv.Value.Mtime, kv.Key);
                File.WriteAllText(Cfg.StateFile + ".new", sb.ToString(), new UTF8Encoding(false));
                if (File.Exists(Cfg.StateFile)) File.Delete(Cfg.StateFile);
                File.Move(Cfg.StateFile + ".new", Cfg.StateFile);
            }
            catch { }
        }
    }

    static class Util
    {
        public static string Sha256(string file)
        {
            using (var sha = SHA256.Create())
            using (var fs = new FileStream(file, FileMode.Open, FileAccess.Read, FileShare.Read, 1 << 20))
            {
                byte[] buf = new byte[1 << 20];
                int n;
                while ((n = fs.Read(buf, 0, buf.Length)) > 0) sha.TransformBlock(buf, 0, n, null, 0);
                sha.TransformFinalBlock(buf, 0, 0);
                var sb = new StringBuilder(64);
                foreach (byte b in sha.Hash) sb.Append(b.ToString("x2"));
                return sb.ToString();
            }
        }

        // An optional pack (a weapon variant) drops this file in with its gamedata.
        public const string VariantMarker = "gamedata/variant_weapons.ltx";

        // -> name of the optional variant installed in the game folder, or null.
        public static string InstalledVariant()
        {
            try
            {
                string file = Local(VariantMarker);
                if (!File.Exists(file)) return null;
                foreach (string raw in File.ReadAllLines(file, Encoding.GetEncoding(1251)))
                {
                    string line = raw.Trim();
                    if (line.Length == 0 || line[0] == ';' || line[0] == '#' || line[0] == '[') continue;
                    int eq = line.IndexOf('=');
                    if (eq <= 0) continue;
                    if (line.Substring(0, eq).Trim().ToLowerInvariant() != "name") continue;
                    string v = line.Substring(eq + 1).Trim();
                    if (v.Length > 0) return v;
                }
            }
            catch { }
            return null;
        }

        public static string Local(string relative)
        {
            return Path.Combine(Cfg.GameRoot, relative.Replace('/', '\\'));
        }

        public static string RawUrl(string relative)
        {
            var sb = new StringBuilder();
            if (Cfg.Base.Length > 0)
                sb.Append(Cfg.Base).Append('/');
            else
                sb.Append("https://raw.githubusercontent.com/").Append(Cfg.Repo)
                  .Append('/').Append(Cfg.Ref).Append('/');
            string[] parts = relative.Split('/');
            for (int i = 0; i < parts.Length; i++)
            {
                if (i > 0) sb.Append('/');
                sb.Append(Uri.EscapeDataString(parts[i]));
            }
            return sb.ToString();
        }

        public static string Mb(long bytes)
        {
            if (bytes >= 1048576L * 1024) return (bytes / 1073741824.0).ToString("0.00", CultureInfo.InvariantCulture) + " ГБ";
            if (bytes >= 1048576L) return (bytes / 1048576.0).ToString("0.0", CultureInfo.InvariantCulture) + " МБ";
            return (bytes / 1024.0).ToString("0.0", CultureInfo.InvariantCulture) + " КБ";
        }

        public static void Unprotect(string file)
        {
            try
            {
                if (File.Exists(file))
                {
                    var attr = File.GetAttributes(file);
                    if ((attr & FileAttributes.ReadOnly) != 0)
                        File.SetAttributes(file, attr & ~FileAttributes.ReadOnly);
                }
            }
            catch { }
        }

        // Scripts the build ships, by bare filename: that is how X-Ray addresses
        // a module, so two files with one name are two modules with one name.
        public static HashSet<string> ScriptNames(Dictionary<string, Entry> manifest)
        {
            var names = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            foreach (var kv in manifest)
            {
                string p = kv.Key;
                if (!p.StartsWith("gamedata/scripts/", StringComparison.OrdinalIgnoreCase)) continue;
                if (!p.EndsWith(".script", StringComparison.OrdinalIgnoreCase)) continue;
                names.Add(System.IO.Path.GetFileName(p));
            }
            return names;
        }

        // Local *.script files that carry a shipped module name from a path the
        // build does not use -- leftovers that shadow the real module.
        public static List<string> ShadowScripts(Dictionary<string, Entry> manifest,
                                                 HashSet<string> keep)
        {
            var found = new List<string>();
            var names = ScriptNames(manifest);
            if (names.Count == 0) return found;
            string root = Local("gamedata/scripts");
            if (!Directory.Exists(root)) return found;
            foreach (string full in Directory.GetFiles(root, "*.script", SearchOption.AllDirectories))
            {
                string rel = full.Substring(Cfg.GameRoot.Length).TrimStart('\\').Replace('\\', '/');
                if (manifest.ContainsKey(rel)) continue;
                if (keep != null && keep.Contains(rel)) continue;
                if (names.Contains(Path.GetFileName(full))) found.Add(rel);
            }
            return found;
        }

        // Keep a copy of whatever is about to be overwritten or deleted, so a bad
        // update can always be undone by hand.
        public static string Backup(string relative, string stamp)
        {
            string local = Local(relative);
            if (!File.Exists(local)) return null;
            string dest = Path.Combine(Path.Combine(Cfg.BackupRoot, stamp),
                                       relative.Replace('/', '\\'));
            Directory.CreateDirectory(Path.GetDirectoryName(dest));
            File.Copy(local, dest, true);
            return dest;
        }
    }

    class Net2
    {
        public static void Init()
        {
            try { ServicePointManager.SecurityProtocol = (SecurityProtocolType)3072; }
            catch { }
            ServicePointManager.Expect100Continue = false;
            ServicePointManager.DefaultConnectionLimit = 8;
        }

        public static string GetString(string url)
        {
            using (var wc = new WebClient())
            {
                wc.Headers["User-Agent"] = "NLC-Launcher/1.0";
                wc.Headers["Cache-Control"] = "no-cache";
                wc.Headers["Pragma"] = "no-cache";
                return Encoding.UTF8.GetString(wc.DownloadData(url));
            }
        }

        public static void GetFile(string url, string dest, Action<long> onChunk, Func<bool> cancelled)
        {
            var req = (HttpWebRequest)WebRequest.Create(url);
            req.UserAgent = "NLC-Launcher/1.0";
            req.Timeout = 30000;
            req.ReadWriteTimeout = 60000;
            using (var resp = (HttpWebResponse)req.GetResponse())
            using (var src = resp.GetResponseStream())
            using (var dst = new FileStream(dest, FileMode.Create, FileAccess.Write, FileShare.None, 1 << 18))
            {
                byte[] buf = new byte[1 << 18];
                int n;
                while ((n = src.Read(buf, 0, buf.Length)) > 0)
                {
                    dst.Write(buf, 0, n);
                    if (onChunk != null) onChunk(n);
                    if (cancelled != null && cancelled()) throw new OperationCanceledException();
                }
            }
        }
    }

    class MainForm : Form
    {
        Label lblStatus;
        Label lblDetail;
        ProgressBar bar;
        TextBox log;
        Button btnUpdate;
        Button btnPlay;
        Button btnCancel;
        Thread worker;
        volatile bool cancelRequested;
        volatile bool confirmed;
        readonly ManualResetEvent answered = new ManualResetEvent(false);

        public MainForm()
        {
            Text = "Апдейтер";
            StartPosition = FormStartPosition.CenterScreen;
            ClientSize = new Size(620, 320);
            MinimumSize = new Size(560, 260);
            Font = new Font("Segoe UI", 9f);

            lblStatus = new Label();
            lblStatus.Text = "Запуск...";
            lblStatus.SetBounds(12, 12, 596, 20);
            lblStatus.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;

            lblDetail = new Label();
            lblDetail.Text = "";
            lblDetail.ForeColor = SystemColors.GrayText;
            lblDetail.SetBounds(12, 34, 596, 18);
            lblDetail.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;

            bar = new ProgressBar();
            bar.SetBounds(12, 56, 596, 18);
            bar.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
            bar.Style = ProgressBarStyle.Marquee;
            bar.MarqueeAnimationSpeed = 30;

            log = new TextBox();
            log.Multiline = true;
            log.ReadOnly = true;
            log.ScrollBars = ScrollBars.Vertical;
            log.BackColor = Color.White;
            log.SetBounds(12, 84, 596, 190);
            log.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;

            btnUpdate = new Button();
            btnUpdate.Text = "Обновить";
            btnUpdate.SetBounds(348, 282, 80, 26);
            btnUpdate.Anchor = AnchorStyles.Bottom | AnchorStyles.Right;
            btnUpdate.Enabled = false;
            btnUpdate.Click += delegate
            {
                btnUpdate.Enabled = false;
                btnPlay.Enabled = false;
                confirmed = true;
                answered.Set();
            };

            btnPlay = new Button();
            btnPlay.Text = "Играть";
            btnPlay.SetBounds(438, 282, 80, 26);
            btnPlay.Anchor = AnchorStyles.Bottom | AnchorStyles.Right;
            btnPlay.Enabled = false;
            btnPlay.Click += delegate { LaunchAndExit(); };

            btnCancel = new Button();
            btnCancel.Text = "Отмена";
            btnCancel.SetBounds(528, 282, 80, 26);
            btnCancel.Anchor = AnchorStyles.Bottom | AnchorStyles.Right;
            btnCancel.Click += delegate
            {
                if (cancelRequested) { Close(); return; }
                cancelRequested = true;
                btnCancel.Enabled = false;
                answered.Set();
                Say("Отмена...");
            };

            Controls.AddRange(new Control[] { lblStatus, lblDetail, bar, log,
                                              btnUpdate, btnPlay, btnCancel });
            Shown += delegate { Start(); };
            FormClosing += delegate { cancelRequested = true; answered.Set(); };
        }

        void Start()
        {
            worker = new Thread(Run);
            worker.IsBackground = true;
            worker.Start();
        }

        void Ui(Action a)
        {
            if (IsDisposed) return;
            try
            {
                if (InvokeRequired) BeginInvoke(a);
                else a();
            }
            catch { }
        }

        void Status(string s) { Ui(delegate { lblStatus.Text = s; }); }
        void Detail(string s) { Ui(delegate { lblDetail.Text = s; }); }

        void Say(string s)
        {
            Ui(delegate { log.AppendText(s + "\r\n"); });
            try
            {
                Directory.CreateDirectory(Cfg.StateDir);
                File.AppendAllText(Cfg.LogFile,
                    DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss") + "  " + s + "\r\n",
                    new UTF8Encoding(false));
            }
            catch { }
        }

        void Marquee(bool on)
        {
            Ui(delegate
            {
                bar.Style = on ? ProgressBarStyle.Marquee : ProgressBarStyle.Continuous;
                if (!on) bar.Value = 0;
            });
        }

        void Progress(int percent)
        {
            Ui(delegate
            {
                if (bar.Style == ProgressBarStyle.Marquee) bar.Style = ProgressBarStyle.Continuous;
                bar.Value = Math.Max(0, Math.Min(100, percent));
            });
        }

        void OfferPlay(string status)
        {
            Ui(delegate
            {
                lblStatus.Text = status;
                bar.Style = ProgressBarStyle.Continuous;
                btnUpdate.Enabled = false;
                btnPlay.Enabled = File.Exists(Util.Local(Cfg.GameExe));
                if (btnPlay.Enabled) btnPlay.Focus();
                btnCancel.Text = "Выход";
                btnCancel.Enabled = true;
                cancelRequested = true;
                if (Cfg.ExitWhenDone) Close();
            });
        }

        // Ask before touching files the updater did not install itself.
        bool Confirm(string status)
        {
            answered.Reset();
            confirmed = false;
            Ui(delegate
            {
                lblStatus.Text = status;
                bar.Style = ProgressBarStyle.Continuous;
                btnUpdate.Enabled = true;
                btnUpdate.Focus();
                btnPlay.Enabled = File.Exists(Util.Local(Cfg.GameExe));
            });
            answered.WaitOne();
            Ui(delegate { btnUpdate.Enabled = false; btnPlay.Enabled = false; });
            return confirmed && !cancelRequested;
        }

        void Run()
        {
            try
            {
                string exe = Util.Local(Cfg.GameExe);
                if (!File.Exists(exe))
                {
                    Say("Не найден " + Cfg.GameExe + ".");
                    Say("Лаунчер должен лежать в корне игры, рядом с папкой bin_x64.");
                    Say("Текущая папка: " + Cfg.GameRoot);
                    OfferPlay("Не найден " + Cfg.GameExe);
                    return;
                }

                if (Cfg.NoUpdate)
                {
                    Say("Проверка обновлений отключена (-noupdate).");
                    LaunchAndExit();
                    return;
                }

                // A development checkout is not a player's install: the working copy is
                // ahead of the repository, and syncing it to the manifest destroys work.
                if (!Cfg.DevOk && Directory.Exists(Path.Combine(Cfg.GameRoot, @"to_git\.git")))
                {
                    Say("Рядом лежит рабочая копия to_git — это папка разработки, не сборка игрока.");
                    Say("Обновление пропущено, чтобы не затереть несохранённые правки (-devok снимает запрет).");
                    OfferPlay("Папка разработки — обновление пропущено");
                    return;
                }

                Status("Проверка обновлений...");
                Say("Репозиторий: " + Cfg.Repo + " (" + Cfg.Ref + ")");

                Dictionary<string, Entry> manifest;
                try
                {
                    string url = Util.RawUrl(Cfg.ManifestName) + "?_=" + DateTime.UtcNow.Ticks;
                    manifest = Manifest.Parse(Net2.GetString(url));
                }
                catch (Exception ex)
                {
                    Say("Не удалось получить список файлов: " + ex.Message);
                    Say("Похоже, нет сети. Игру можно запустить без обновления.");
                    OfferPlay("Обновление недоступно");
                    return;
                }

                if (manifest.Count == 0)
                {
                    Say("Манифест пуст или повреждён — обновление пропущено.");
                    OfferPlay("Обновление недоступно");
                    return;
                }
                Say("Версия сборки: " + (Manifest.Version.Length > 0 ? Manifest.Version : "?")
                    + ", файлов в сборке: " + manifest.Count);

                // Files an installed optional variant owns are left alone: the build
                // ships the stock weapons, watch and hands, and putting those back is
                // exactly what uninstalls the variant.
                var keep = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
                string variant = Util.InstalledVariant();
                if (variant != null && Cfg.NoKeep)
                    Say("-nokeep: файлы варианта " + variant
                        + " будут заменены версиями из сборки.");
                else if (variant != null)
                {
                    HashSet<string> owned;
                    if (Manifest.Keep.TryGetValue(variant, out owned))
                        foreach (string p in owned) keep.Add(p);
                    else
                        Say("Вариант " + variant + " не описан в манифесте — его файлы не защищены.");
                }

                var state = State.Load();
                var todo = new List<Entry>();
                var removed = new List<string>();
                long todoBytes = 0;
                int kept = 0;

                Status("Сверка файлов...");
                Marquee(false);
                int i = 0;
                foreach (var kv in manifest)
                {
                    if (cancelRequested) { Cancelled(); return; }
                    i++;
                    if ((i & 63) == 0)
                    {
                        Progress((int)(100L * i / manifest.Count));
                        Detail(i + " / " + manifest.Count);
                    }
                    Entry e = kv.Value;
                    if (keep.Contains(e.Path)) { kept++; continue; }
                    string local = Util.Local(e.Path);
                    if (!File.Exists(local))
                    {
                        e.LocalExists = false;
                        e.Ours = false;
                        todo.Add(e);
                        todoBytes += e.Size;
                        continue;
                    }

                    e.LocalExists = true;
                    var fi = new FileInfo(local);
                    StateRec rec;
                    bool known = state.TryGetValue(e.Path, out rec);
                    string sha;
                    if (known && rec.Size == fi.Length && rec.Mtime == fi.LastWriteTimeUtc.Ticks)
                    {
                        sha = rec.Sha;
                    }
                    else
                    {
                        try { sha = Util.Sha256(local); }
                        catch
                        {
                            e.Ours = false;
                            todo.Add(e);
                            todoBytes += e.Size;
                            continue;
                        }
                        state[e.Path] = new StateRec
                        {
                            Sha = sha,
                            Size = fi.Length,
                            Mtime = fi.LastWriteTimeUtc.Ticks
                        };
                    }
                    // "ours" == this exact file is what the updater last installed
                    e.Ours = known && string.Equals(rec.Sha, sha, StringComparison.OrdinalIgnoreCase);
                    if (!string.Equals(sha, e.Sha, StringComparison.OrdinalIgnoreCase))
                    {
                        todo.Add(e);
                        todoBytes += e.Size;
                    }
                }

                foreach (var kv in state)
                {
                    if (manifest.ContainsKey(kv.Key)) continue;
                    if (keep.Contains(kv.Key)) continue;
                    string local = Util.Local(kv.Key);
                    if (!File.Exists(local)) continue;
                    try
                    {
                        var fi = new FileInfo(local);
                        string sha = (kv.Value.Size == fi.Length && kv.Value.Mtime == fi.LastWriteTimeUtc.Ticks)
                            ? kv.Value.Sha : Util.Sha256(local);
                        if (string.Equals(sha, kv.Value.Sha, StringComparison.OrdinalIgnoreCase))
                            removed.Add(kv.Key);
                    }
                    catch { }
                }

                var shadows = Util.ShadowScripts(manifest, keep);

                Detail("");
                if (keep.Count > 0)
                    Say("Вариант оружия: " + variant + " — " + kept
                        + " файл(ов) пропущено (-nokeep снимает защиту).");
                if (todo.Count == 0 && removed.Count == 0 && shadows.Count == 0)
                {
                    State.Save(state);
                    Progress(100);
                    Status("Сборка актуальна — запуск игры");
                    Say("Обновлений нет.");
                    LaunchAndExit();
                    return;
                }

                // Files that already exist and were not installed by the updater are
                // somebody's own: local edits, another mod, a variant pack. Overwriting
                // them silently is how an update eats work, so it needs a yes.
                var foreign = new List<Entry>();
                foreach (Entry e in todo)
                    if (e.LocalExists && !e.Ours) foreign.Add(e);

                Say("К загрузке: " + todo.Count + " файл(ов), " + Util.Mb(todoBytes)
                    + (removed.Count > 0 ? "; удалить: " + removed.Count : ""));

                if (shadows.Count > 0)
                {
                    Say("Найдены лишние копии модулей сборки — X-Ray адресует скрипт по имени файла,");
                    Say("поэтому такие копии подменяют собой настоящие модули и ломают игру:");
                    foreach (string p in shadows) Say("  " + p);
                    Say("Они будут убраны в appdata\\updater\\backup.");
                }

                if ((foreign.Count > 0 || shadows.Count > 0) && !Cfg.AssumeYes)
                {
                    if (foreign.Count > 0)
                    {
                        Say("Из них " + foreign.Count + " файл(ов) в папке игры отличаются, и ставил их не апдейтер:");
                        int shown = 0;
                        foreach (Entry e in foreign)
                        {
                            if (shown++ == 15) { Say("  ... и ещё " + (foreign.Count - 15)); break; }
                            Say("  " + e.Path);
                    }
                    Say("Они будут заменены версиями из сборки. Копии сохранятся в appdata\\updater\\backup.");
                    }
                    if (!Confirm("Подтвердите изменение " + (foreign.Count + shadows.Count) + " файл(ов)"))
                    {
                        Say("Обновление отменено пользователем.");
                        OfferPlay("Обновление отменено");
                        return;
                    }
                }

                Status("Загрузка обновления...");
                string stamp = DateTime.Now.ToString("yyyyMMdd-HHmmss");

                Directory.CreateDirectory(Cfg.TempDir);
                long done = 0;
                int written = 0;
                string tmp = Path.Combine(Cfg.TempDir, "part.bin");
                foreach (Entry e in todo)
                {
                    if (cancelRequested) { State.Save(state); Cancelled(); return; }
                    Detail(e.Path);
                    long fileBase = done;
                    bool ok = false;
                    string lastError = "";
                    for (int attempt = 1; attempt <= 3 && !ok; attempt++)
                    {
                        try
                        {
                            long got = 0;
                            Net2.GetFile(Util.RawUrl(e.Path), tmp,
                                delegate(long n)
                                {
                                    got += n;
                                    if (todoBytes > 0)
                                        Progress((int)(100L * (fileBase + got) / todoBytes));
                                },
                                delegate { return cancelRequested; });
                            string sha = Util.Sha256(tmp);
                            if (!string.Equals(sha, e.Sha, StringComparison.OrdinalIgnoreCase))
                            {
                                lastError = "контрольная сумма не совпала";
                                Thread.Sleep(500 * attempt);
                                continue;
                            }
                            string local = Util.Local(e.Path);
                            if (e.LocalExists && !e.Ours)
                            {
                                try { Util.Backup(e.Path, stamp); }
                                catch (Exception bex) { Say("не удалось сохранить копию " + e.Path + ": " + bex.Message); }
                            }
                            Directory.CreateDirectory(Path.GetDirectoryName(local));
                            Util.Unprotect(local);
                            File.Copy(tmp, local, true);
                            var fi = new FileInfo(local);
                            state[e.Path] = new StateRec
                            {
                                Sha = e.Sha,
                                Size = fi.Length,
                                Mtime = fi.LastWriteTimeUtc.Ticks
                            };
                            ok = true;
                        }
                        catch (OperationCanceledException) { State.Save(state); Cancelled(); return; }
                        catch (Exception ex) { lastError = ex.Message; Thread.Sleep(500 * attempt); }
                    }
                    done += e.Size;
                    Progress(todoBytes > 0 ? (int)(100L * done / todoBytes) : 100);
                    if (!ok) Say("ОШИБКА: " + e.Path + " — " + lastError);
                    else
                    {
                        written++;
                        if ((written & 31) == 0) State.Save(state);
                    }
                }

                foreach (string path in removed)
                {
                    try
                    {
                        string local = Util.Local(path);
                        Util.Backup(path, stamp);
                        Util.Unprotect(local);
                        File.Delete(local);
                        state.Remove(path);
                        Say("удалён " + path);
                    }
                    catch (Exception ex) { Say("не удалось удалить " + path + ": " + ex.Message); }
                }

                foreach (string path in shadows)
                {
                    try
                    {
                        string local = Util.Local(path);
                        Util.Backup(path, stamp);
                        Util.Unprotect(local);
                        File.Delete(local);
                        state.Remove(path);
                        Say("убрана лишняя копия модуля: " + path);
                    }
                    catch (Exception ex) { Say("не удалось убрать " + path + ": " + ex.Message); }
                }

                State.Save(state);
                try { Directory.Delete(Cfg.TempDir, true); }
                catch { }

                string backupDir = Path.Combine(Cfg.BackupRoot, stamp);
                if (Directory.Exists(backupDir))
                    Say("Копии заменённых файлов: appdata\\updater\\backup\\" + stamp);

                int failed = todo.Count - written;
                Detail("");
                if (failed > 0)
                {
                    Say("Не скачано файлов: " + failed + ". Запустите лаунчер ещё раз.");
                    OfferPlay("Обновление завершилось с ошибками");
                    return;
                }

                Progress(100);
                Status("Обновление установлено — запуск игры");
                Say("Готово: обновлено " + written + " файл(ов).");
                LaunchAndExit();
            }
            catch (Exception ex)
            {
                Say("Сбой: " + ex.Message);
                OfferPlay("Сбой обновления");
            }
        }

        void Cancelled()
        {
            Say("Обновление прервано.");
            OfferPlay("Обновление прервано");
        }

        void LaunchAndExit()
        {
            if (Cfg.NoLaunch)
            {
                OfferPlay("Готово (запуск отключён)");
                return;
            }
            try
            {
                string exe = Util.Local(Cfg.GameExe);
                var psi = new ProcessStartInfo(exe);
                psi.WorkingDirectory = Path.GetDirectoryName(exe);
                psi.UseShellExecute = true;
                if (Cfg.ExtraArgs.Length > 0) psi.Arguments = Cfg.ExtraArgs;
                Process.Start(psi);
            }
            catch (Exception ex)
            {
                Say("Не удалось запустить игру: " + ex.Message);
                OfferPlay("Ошибка запуска");
                return;
            }
            Ui(delegate { Close(); });
        }
    }

    static class Program
    {
        [STAThread]
        static void Main(string[] args)
        {
            Cfg.GameRoot = AppDomain.CurrentDomain.BaseDirectory.TrimEnd('\\');
            Cfg.LoadIni();
            Cfg.ParseArgs(args);
            Net2.Init();
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new MainForm());
        }
    }
}
