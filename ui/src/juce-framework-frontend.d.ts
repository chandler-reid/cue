// Minimal types for the parts of JUCE's frontend helper we use.
// Source: JUCE/modules/juce_gui_extra/native/javascript/index.js
declare module "juce-framework-frontend" {
  export function getNativeFunction(name: string): (...args: unknown[]) => Promise<unknown>;
}
