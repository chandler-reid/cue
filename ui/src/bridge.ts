// The only module that talks to the JUCE backend.
//
// Inside the plugin's WebView, JUCE injects `window.__JUCE__` before page
// scripts run. In a plain browser (Vite dev server) it doesn't exist, and we
// fall back to an in-memory mock so the UI can be developed without C++.

export type Note = { id: number; text: string };

// Mirrors NoteBridge::handleAddNote in source/NoteBridge.cpp.
export type AddNoteResult =
  | { ok: true; note: Note }
  | { ok: false; error: string };

export interface Bridge {
  readonly kind: "juce" | "mock";
  addNote(text: string): Promise<AddNoteResult>;
}

declare global {
  interface Window {
    __JUCE__?: unknown;
  }
}

// JUCE's promises never reject: if the backend never calls its completion
// callback the call would hang forever. Surface that as an error instead.
const CALL_TIMEOUT_MS = 2000;

function withTimeout<T>(promise: Promise<T>, name: string): Promise<T> {
  return new Promise((resolve, reject) => {
    const timer = setTimeout(
      () => reject(new Error(`Native call '${name}' timed out`)),
      CALL_TIMEOUT_MS,
    );
    promise.then(
      (value) => {
        clearTimeout(timer);
        resolve(value);
      },
      (err) => {
        clearTimeout(timer);
        reject(err);
      },
    );
  });
}

// Never trust the shape of what crosses the boundary, even from our own C++.
function parseAddNoteResult(value: unknown): AddNoteResult {
  if (typeof value === "object" && value !== null) {
    const v = value as Record<string, unknown>;
    if (v.ok === true && typeof v.note === "object" && v.note !== null) {
      const n = v.note as Record<string, unknown>;
      if (typeof n.id === "number" && typeof n.text === "string")
        return { ok: true, note: { id: n.id, text: n.text } };
    }
    if (v.ok === false && typeof v.error === "string")
      return { ok: false, error: v.error };
  }
  return { ok: false, error: "Malformed response from backend" };
}

async function juceBridge(): Promise<Bridge> {
  // Dynamic import: JUCE's helper touches window.__JUCE__ at module load, so a
  // static import would crash the page in a plain browser.
  const juce = await import("juce-framework-frontend");
  const addNoteNative = juce.getNativeFunction("addNote");

  return {
    kind: "juce",
    async addNote(text) {
      return parseAddNoteResult(await withTimeout(addNoteNative(text), "addNote"));
    },
  };
}

// Keep these rules in sync with NoteBridge.cpp so the mock is a faithful stand-in.
export const MAX_NOTE_LENGTH = 1000;

function mockBridge(): Bridge {
  let nextId = 1;
  return {
    kind: "mock",
    async addNote(text) {
      const trimmed = text.trim();
      if (trimmed.length === 0) return { ok: false, error: "Note is empty" };
      // Count code points ([...s]), not UTF-16 units (s.length), to match juce::String::length().
      if ([...trimmed].length > MAX_NOTE_LENGTH) return { ok: false, error: "Note is too long" };
      return { ok: true, note: { id: nextId++, text: trimmed } };
    },
  };
}

export async function getBridge(): Promise<Bridge> {
  return window.__JUCE__ !== undefined ? juceBridge() : mockBridge();
}
