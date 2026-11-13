import { render } from "preact";
import { App } from "./App";
import { getBridge } from "./bridge";

const root = document.getElementById("app");
if (root) {
  getBridge().then(
    (bridge) => render(<App bridge={bridge} />, root),
    (err) => {
      root.textContent = `Failed to connect to backend: ${String(err)}`;
    },
  );
}
