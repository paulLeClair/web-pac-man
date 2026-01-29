import type { Component } from 'solid-js';
import {createWS, createWSState} from "@solid-primitives/websocket";

const App: Component = () => {
  const ws = createWS("ws://127.0.0.1:80")
  const state = createWSState(ws)
  const states = ["Connecting", "Connected", "Disconnecting", "Disconnected"];

  ws.send("Hello from client!")


  return (
    <p class="text-4xl text-green-700 text-center py-20">Hello tailwind!</p>
  );
};

export default App;
