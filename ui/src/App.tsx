import { useState } from "preact/hooks";
import type { Bridge, Note } from "./bridge";

export function App({ bridge }: { bridge: Bridge }) {
  const [text, setText] = useState("");
  const [notes, setNotes] = useState<Note[]>([]);
  const [error, setError] = useState<string | null>(null);
  const [busy, setBusy] = useState(false);

  async function submit(e: Event) {
    e.preventDefault();
    setBusy(true);
    try {
      const result = await bridge.addNote(text);
      if (result.ok) {
        setNotes((prev) => [...prev, result.note]);
        setText("");
        setError(null);
      } else {
        setError(result.error);
      }
    } catch (err) {
      setError(String(err));
    } finally {
      setBusy(false);
    }
  }

  return (
    <main style={{ fontFamily: "system-ui, sans-serif", padding: "16px" }}>
      <h1 style={{ fontSize: "18px" }}>Cue</h1>
      <p data-testid="backend" style={{ fontSize: "12px", opacity: 0.7 }}>
        Backend: {bridge.kind}
      </p>
      <form onSubmit={submit} style={{ display: "flex", gap: "8px" }}>
        <input
          value={text}
          onInput={(e) => setText(e.currentTarget.value)}
          placeholder="Write a note…"
          aria-label="Note text"
          style={{ flex: 1 }}
        />
        <button type="submit" disabled={busy}>
          Add
        </button>
      </form>
      {error && (
        <p role="alert" style={{ color: "crimson" }}>
          {error}
        </p>
      )}
      <ul>
        {notes.map((n) => (
          <li key={n.id}>
            #{n.id}: {n.text}
          </li>
        ))}
      </ul>
    </main>
  );
}
