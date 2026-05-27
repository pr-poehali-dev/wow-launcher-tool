import { useState, useRef, useCallback } from "react";
import Icon from "@/components/ui/icon";

interface LogEntry {
  id: number;
  time: string;
  message: string;
  status: "success" | "error" | "info" | "warning";
}

const statusColors = {
  success: "text-green-400",
  error: "text-red-400",
  info: "text-blue-400",
  warning: "text-yellow-400",
};

const statusIcons: Record<string, string> = {
  success: "CheckCircle",
  error: "XCircle",
  info: "Info",
  warning: "AlertTriangle",
};

const statusLabels = {
  success: "OK",
  error: "ERR",
  info: "INF",
  warning: "WRN",
};

export default function Index() {
  const [exePath, setExePath] = useState("");
  const [copyCount, setCopyCount] = useState(1);
  const [logs, setLogs] = useState<LogEntry[]>([
    {
      id: 1,
      time: new Date().toLocaleTimeString("ru-RU"),
      message: "Лаунчер инициализирован. Готов к работе.",
      status: "info",
    },
  ]);
  const [isRunning, setIsRunning] = useState(false);
  const [logFilter, setLogFilter] = useState<"all" | LogEntry["status"]>("all");
  const logEndRef = useRef<HTMLDivElement>(null);
  const logIdRef = useRef(2);

  const addLog = useCallback(
    (message: string, status: LogEntry["status"] = "info") => {
      const entry: LogEntry = {
        id: logIdRef.current++,
        time: new Date().toLocaleTimeString("ru-RU"),
        message,
        status,
      };
      setLogs((prev) => [...prev.slice(-199), entry]);
      setTimeout(() => {
        logEndRef.current?.scrollIntoView({ behavior: "smooth" });
      }, 50);
    },
    []
  );

  const handleBrowse = () => {
    const mockPath =
      "C:\\Program Files (x86)\\World of Warcraft\\_retail_\\Wow.exe";
    setExePath(mockPath);
    addLog(`Выбран файл: ${mockPath}`, "info");
  };

  const handleLaunch = () => {
    if (!exePath.trim()) {
      addLog("Ошибка: путь к exe-файлу не указан.", "error");
      return;
    }
    if (copyCount < 1 || copyCount > 50) {
      addLog("Ошибка: количество копий должно быть от 1 до 50.", "error");
      return;
    }

    setIsRunning(true);
    addLog(`Запуск ${copyCount} копий: ${exePath}`, "info");

    let launched = 0;
    const interval = setInterval(() => {
      launched++;
      const pid = Math.floor(Math.random() * 9000) + 1000;
      addLog(
        `[PID: ${pid}] Экземпляр #${launched} запущен успешно.`,
        "success"
      );

      if (launched >= copyCount) {
        clearInterval(interval);
        addLog(
          `Все ${copyCount} ${copyCount === 1 ? "копия" : "копий"} запущены. Проверка log.conf...`,
          "info"
        );
        setTimeout(() => {
          addLog(
            "Файл log.conf найден. Загрузка учётных данных WoW...",
            "info"
          );
          setTimeout(() => {
            addLog(
              "Автологин выполнен для всех экземпляров.",
              "success"
            );
            setIsRunning(false);
          }, 800);
        }, 500);
      }
    }, 300);
  };

  const handleClearLogs = () => {
    setLogs([]);
    addLog("Лог очищен.", "info");
  };

  const filteredLogs =
    logFilter === "all"
      ? logs
      : logs.filter((l) => l.status === logFilter);

  const logCounts = logs.reduce(
    (acc, l) => ({ ...acc, [l.status]: (acc[l.status] || 0) + 1 }),
    {} as Record<string, number>
  );

  return (
    <div className="min-h-screen mesh-bg flex flex-col">
      {/* Header */}
      <header className="relative border-b border-[var(--wow-border)] px-6 py-4">
        <div
          className="absolute inset-0 opacity-30"
          style={{
            background:
              "linear-gradient(90deg, transparent 0%, rgba(74,158,255,0.08) 50%, transparent 100%)",
          }}
        />
        <div className="relative flex items-center justify-between max-w-5xl mx-auto">
          <div className="flex items-center gap-3">
            <div
              className="w-10 h-10 rounded-xl flex items-center justify-center"
              style={{
                background:
                  "linear-gradient(135deg, #1e3a5f 0%, #0d2040 100%)",
                border: "1px solid rgba(74,158,255,0.4)",
              }}
            >
              <Icon name="Gamepad2" size={20} className="text-blue-400" />
            </div>
            <div>
              <h1 className="font-oswald text-xl font-semibold tracking-widest gradient-text-gold uppercase">
                WoW Launcher
              </h1>
              <p className="text-xs font-mono-plex text-slate-500 tracking-wider">
                v1.0.0 — Multi-Instance Manager
              </p>
            </div>
          </div>
          <div className="flex items-center gap-2">
            <div
              className={`w-2 h-2 rounded-full ${
                isRunning ? "bg-green-400 pulse-active" : "bg-slate-600"
              }`}
            />
            <span className="font-mono-plex text-xs text-slate-500">
              {isRunning ? "RUNNING" : "IDLE"}
            </span>
          </div>
        </div>
      </header>

      <main className="flex-1 max-w-5xl mx-auto w-full px-6 py-8 space-y-6">
        {/* EXE Path Block */}
        <div className="card-glass rounded-2xl p-6 animate-fade-in-up">
          <div className="flex items-center gap-2 mb-4">
            <div
              className="w-1 h-5 rounded-full"
              style={{
                background:
                  "linear-gradient(180deg, var(--wow-gold) 0%, transparent 100%)",
              }}
            />
            <h2 className="font-oswald text-base font-medium tracking-widest text-slate-300 uppercase">
              Целевой файл
            </h2>
          </div>

          <div className="flex gap-3">
            <div className="flex-1 relative">
              <Icon
                name="FolderOpen"
                size={16}
                className="absolute left-3 top-1/2 -translate-y-1/2 text-slate-500"
              />
              <input
                type="text"
                value={exePath}
                onChange={(e) => setExePath(e.target.value)}
                placeholder="C:\Program Files\...\Wow.exe"
                className="input-wow w-full rounded-xl pl-9 pr-4 py-3 font-mono-plex text-sm"
              />
            </div>
            <button
              onClick={handleBrowse}
              className="btn-browse rounded-xl px-5 py-3 font-rajdhani font-semibold text-sm tracking-widest text-yellow-400 uppercase whitespace-nowrap flex items-center gap-2"
            >
              <Icon name="FolderSearch" size={16} />
              Обзор
            </button>
          </div>

          {exePath && (
            <div className="mt-3 flex items-center gap-2">
              <Icon
                name="CheckCircle"
                size={14}
                className="text-green-400 flex-shrink-0"
              />
              <span className="font-mono-plex text-xs text-green-400 truncate">
                {exePath}
              </span>
            </div>
          )}
        </div>

        {/* Launch Controls */}
        <div className="card-glass rounded-2xl p-6 animate-fade-in-up delay-100">
          <div className="flex items-center gap-2 mb-4">
            <div
              className="w-1 h-5 rounded-full"
              style={{
                background:
                  "linear-gradient(180deg, var(--wow-blue) 0%, transparent 100%)",
              }}
            />
            <h2 className="font-oswald text-base font-medium tracking-widest text-slate-300 uppercase">
              Параметры запуска
            </h2>
          </div>

          <div className="flex gap-4 items-end">
            <div className="w-48">
              <label className="block font-rajdhani text-xs font-medium text-slate-500 uppercase tracking-widest mb-2">
                Количество копий
              </label>
              <div
                className="flex items-center rounded-xl overflow-hidden border border-[var(--wow-border)]"
              >
                <button
                  onClick={() => setCopyCount((v) => Math.max(1, v - 1))}
                  className="w-10 h-11 flex items-center justify-center text-slate-400 hover:text-white transition-colors flex-shrink-0"
                  style={{ background: "rgba(8,12,20,0.8)" }}
                >
                  <Icon name="Minus" size={14} />
                </button>
                <input
                  type="number"
                  min={1}
                  max={50}
                  value={copyCount}
                  onChange={(e) =>
                    setCopyCount(
                      Math.max(1, Math.min(50, parseInt(e.target.value) || 1))
                    )
                  }
                  className="w-full h-11 text-center input-wow font-oswald text-lg font-medium border-0 border-x border-[var(--wow-border)] rounded-none"
                  style={{
                    WebkitAppearance: "none",
                    MozAppearance: "textfield",
                  } as React.CSSProperties}
                />
                <button
                  onClick={() => setCopyCount((v) => Math.min(50, v + 1))}
                  className="w-10 h-11 flex items-center justify-center text-slate-400 hover:text-white transition-colors flex-shrink-0"
                  style={{ background: "rgba(8,12,20,0.8)" }}
                >
                  <Icon name="Plus" size={14} />
                </button>
              </div>
            </div>

            <button
              onClick={handleLaunch}
              disabled={isRunning}
              className="flex-1 btn-launch rounded-xl py-3 font-oswald font-semibold text-base tracking-widest text-white uppercase flex items-center justify-center gap-3 disabled:opacity-50 disabled:cursor-not-allowed"
            >
              {isRunning ? (
                <>
                  <Icon name="Loader" size={18} className="animate-spin" />
                  Запуск...
                </>
              ) : (
                <>
                  <Icon name="Play" size={18} />
                  Запустить{" "}
                  {copyCount === 1
                    ? "1 копию"
                    : copyCount < 5
                    ? `${copyCount} копии`
                    : `${copyCount} копий`}
                </>
              )}
            </button>
          </div>
        </div>

        {/* log.conf Info Block */}
        <div
          className="rounded-xl px-5 py-3 animate-fade-in-up delay-200 flex items-center gap-4"
          style={{
            background:
              "linear-gradient(135deg, rgba(155,109,255,0.08) 0%, rgba(74,158,255,0.05) 100%)",
            border: "1px solid rgba(155,109,255,0.25)",
          }}
        >
          <Icon
            name="FileKey"
            size={20}
            className="text-purple-400 flex-shrink-0"
          />
          <div className="flex-1">
            <p className="font-rajdhani font-semibold text-sm text-purple-300">
              Конфигурация log.conf
            </p>
            <p className="font-mono-plex text-xs text-slate-500 mt-0.5">
              При отсутствии файл будет создан автоматически с полями: login,
              password, realm, region
            </p>
          </div>
          <div
            className="flex items-center gap-1.5 px-3 py-1 rounded-lg flex-shrink-0"
            style={{
              background: "rgba(34,197,94,0.1)",
              border: "1px solid rgba(34,197,94,0.2)",
            }}
          >
            <div className="w-1.5 h-1.5 rounded-full bg-green-400" />
            <span className="font-mono-plex text-xs text-green-400">FOUND</span>
          </div>
        </div>

        {/* Log Block */}
        <div className="card-glass rounded-2xl animate-fade-in-up delay-300">
          {/* Log Header */}
          <div className="flex items-center justify-between px-6 py-4 border-b border-[var(--wow-border)] flex-wrap gap-2">
            <div className="flex items-center gap-2">
              <div
                className="w-1 h-5 rounded-full"
                style={{
                  background:
                    "linear-gradient(180deg, var(--wow-green) 0%, transparent 100%)",
                }}
              />
              <h2 className="font-oswald text-base font-medium tracking-widest text-slate-300 uppercase">
                Журнал событий
              </h2>
              <span className="font-mono-plex text-xs text-slate-600 ml-1">
                ({logs.length})
              </span>
            </div>
            <div className="flex items-center gap-1 flex-wrap">
              {(
                ["all", "success", "error", "warning", "info"] as const
              ).map((f) => (
                <button
                  key={f}
                  onClick={() => setLogFilter(f)}
                  className={`font-mono-plex text-xs px-2.5 py-1 rounded-lg transition-all ${
                    logFilter === f
                      ? "text-white"
                      : "text-slate-600 hover:text-slate-400"
                  }`}
                  style={
                    logFilter === f
                      ? {
                          background:
                            f === "success"
                              ? "rgba(34,197,94,0.2)"
                              : f === "error"
                              ? "rgba(239,68,68,0.2)"
                              : f === "warning"
                              ? "rgba(240,180,41,0.2)"
                              : "rgba(74,158,255,0.2)",
                          border: "1px solid rgba(74,158,255,0.2)",
                        }
                      : {}
                  }
                >
                  {f === "all"
                    ? `ALL (${logs.length})`
                    : `${f.toUpperCase()} (${logCounts[f] || 0})`}
                </button>
              ))}
              <button
                onClick={handleClearLogs}
                className="font-mono-plex text-xs text-slate-600 hover:text-red-400 px-2.5 py-1 rounded-lg transition-colors ml-1"
                title="Очистить лог"
              >
                <Icon name="Trash2" size={13} />
              </button>
            </div>
          </div>

          {/* Log Entries */}
          <div
            className="h-72 overflow-y-auto px-4 py-3 space-y-1"
          >
            {filteredLogs.length === 0 ? (
              <div className="h-full flex items-center justify-center text-slate-600 text-sm font-mono-plex">
                Нет записей
              </div>
            ) : (
              filteredLogs.map((entry) => (
                <div
                  key={entry.id}
                  className="log-entry flex items-start gap-3 py-1.5 px-3 rounded-lg hover:bg-white/[0.02] transition-colors"
                >
                  <span className="font-mono-plex text-slate-600 text-xs pt-0.5 flex-shrink-0 w-20">
                    {entry.time}
                  </span>
                  <span
                    className={`font-mono-plex text-xs font-medium flex-shrink-0 w-7 pt-0.5 ${
                      statusColors[entry.status]
                    }`}
                  >
                    {statusLabels[entry.status]}
                  </span>
                  <Icon
                    name={statusIcons[entry.status]}
                    fallback="Info"
                    size={13}
                    className={`${statusColors[entry.status]} flex-shrink-0 mt-0.5`}
                  />
                  <span className="font-mono-plex text-xs text-slate-300 leading-5 break-all">
                    {entry.message}
                  </span>
                </div>
              ))
            )}
            <div ref={logEndRef} />
          </div>

          {/* Log Footer */}
          <div className="px-6 py-3 border-t border-[var(--wow-border)] flex items-center justify-between">
            <span className="font-mono-plex text-xs text-slate-600">
              Последнее обновление: {logs[logs.length - 1]?.time || "—"}
            </span>
            <div className="flex gap-3">
              <span className="font-mono-plex text-xs text-green-500">
                ✓ {logCounts.success || 0}
              </span>
              <span className="font-mono-plex text-xs text-red-500">
                ✗ {logCounts.error || 0}
              </span>
              <span className="font-mono-plex text-xs text-yellow-500">
                ⚠ {logCounts.warning || 0}
              </span>
            </div>
          </div>
        </div>
      </main>

      {/* Footer */}
      <footer className="border-t border-[var(--wow-border)] px-6 py-3">
        <div className="max-w-5xl mx-auto flex items-center justify-between">
          <span className="font-mono-plex text-xs text-slate-700">
            WoW Multi-Instance Launcher © 2026
          </span>
          <span className="font-mono-plex text-xs text-slate-700">
            log.conf · C:\Users\...\AppData\Roaming\WowLauncher\
          </span>
        </div>
      </footer>
    </div>
  );
}
