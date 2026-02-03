// package: 
// file: game_state.proto

import * as jspb from "google-protobuf";

export class GameStateMessage extends jspb.Message {
  getCurrentmode(): number;
  setCurrentmode(value: number): void;

  getPacmanpositionx(): number;
  setPacmanpositionx(value: number): void;

  getPacmanpositiony(): number;
  setPacmanpositiony(value: number): void;

  getPacmanorientation(): number;
  setPacmanorientation(value: number): void;

  getPacmanischomping(): boolean;
  setPacmanischomping(value: boolean): void;

  getGhostsarescattering(): boolean;
  setGhostsarescattering(value: boolean): void;

  getPinkypositionx(): number;
  setPinkypositionx(value: number): void;

  getPinkypositiony(): number;
  setPinkypositiony(value: number): void;

  getPinkyorientation(): number;
  setPinkyorientation(value: number): void;

  getPinkyisdead(): boolean;
  setPinkyisdead(value: boolean): void;

  getInkypositionx(): number;
  setInkypositionx(value: number): void;

  getInkypositiony(): number;
  setInkypositiony(value: number): void;

  getInkyorientation(): number;
  setInkyorientation(value: number): void;

  getInkyisdead(): boolean;
  setInkyisdead(value: boolean): void;

  getBlinkypositionx(): number;
  setBlinkypositionx(value: number): void;

  getBlinkypositiony(): number;
  setBlinkypositiony(value: number): void;

  getBlinkyorientation(): number;
  setBlinkyorientation(value: number): void;

  getBlinkyisdead(): boolean;
  setBlinkyisdead(value: boolean): void;

  getClydepositionx(): number;
  setClydepositionx(value: number): void;

  getClydepositiony(): number;
  setClydepositiony(value: number): void;

  getClydeorientation(): number;
  setClydeorientation(value: number): void;

  getClydeisdead(): boolean;
  setClydeisdead(value: boolean): void;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): GameStateMessage.AsObject;
  static toObject(includeInstance: boolean, msg: GameStateMessage): GameStateMessage.AsObject;
  static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
  static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
  static serializeBinaryToWriter(message: GameStateMessage, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): GameStateMessage;
  static deserializeBinaryFromReader(message: GameStateMessage, reader: jspb.BinaryReader): GameStateMessage;
}

export namespace GameStateMessage {
  export type AsObject = {
    currentmode: number,
    pacmanpositionx: number,
    pacmanpositiony: number,
    pacmanorientation: number,
    pacmanischomping: boolean,
    ghostsarescattering: boolean,
    pinkypositionx: number,
    pinkypositiony: number,
    pinkyorientation: number,
    pinkyisdead: boolean,
    inkypositionx: number,
    inkypositiony: number,
    inkyorientation: number,
    inkyisdead: boolean,
    blinkypositionx: number,
    blinkypositiony: number,
    blinkyorientation: number,
    blinkyisdead: boolean,
    clydepositionx: number,
    clydepositiony: number,
    clydeorientation: number,
    clydeisdead: boolean,
  }
}

