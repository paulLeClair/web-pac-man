import type { Component } from 'solid-js';
import {createWS, createWSState} from "@solid-primitives/websocket";
import Board from "./Board/Board";

const App: Component = () => {
  const ws = createWS("ws://127.0.0.1:80")
  const state = createWSState(ws)
  const states = ["Connecting", "Connected", "Disconnecting", "Disconnected"];

  ws.send("Hello from client!") // TODO -> break off a little handshake procedure for init;


  return (
      <Board/>
  );
};

export default App;
