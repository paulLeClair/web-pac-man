import type { Component } from 'solid-js';
import {createWS, createWSState} from "@solid-primitives/websocket";
import Board from "./Board/Board";
import Pacman from "./Pacman/Pacman";
import Ghost from "./Ghost/Ghost";
import {GhostName} from "./Ghost/Ghost";

// TODO -> move this to a defines file probably


const App: Component = () => {
  const ws = createWS("ws://127.0.0.1:80")
  const state = createWSState(ws)
  const states = ["Connecting", "Connected", "Disconnecting", "Disconnected"];

  ws.send("Hello from client!") // TODO -> break off a little handshake procedure for init;

  return (
      <div>
        <Board/>
        <Pacman/>
        <Ghost ghostName={GhostName.PINKY} />
        <Ghost ghostName={GhostName.INKY} />
        <Ghost ghostName={GhostName.BLINKY} />
        <Ghost ghostName={GhostName.CLYDE} />
      </div>
  );
};

export default App;
