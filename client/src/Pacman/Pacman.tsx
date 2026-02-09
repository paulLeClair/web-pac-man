import {Component, createMemo, createSignal} from 'solid-js';
import './Pacman.css'
import {Direction, EntityState, SideOfBoardPadding, TopOfBoardPadding} from "../App";

enum Orientation {
  UP = "orientation-up",
  DOWN = "orientation-down",
  LEFT = "orientation-left",
  RIGHT = "orientation-right"
}

export interface PacmanState extends EntityState {
  isChomping: boolean
}

export interface PacmanProps {
  pacmanStateAccessor: () => PacmanState,
  scaleFactor: number
}

const Pacman: Component<PacmanProps> = (props) => {
  const positionStyles = createMemo(() => {
    const s = props.pacmanStateAccessor();
    return {
      top: `${props.scaleFactor * (s.y + TopOfBoardPadding)}px`,
      left: `${props.scaleFactor * (s.x + SideOfBoardPadding)}px`,
    };
  });

  const pacmanOrientation = createMemo(() => {
    switch (props.pacmanStateAccessor().orientation) {
      case Direction.UP: return Orientation.UP;
      case Direction.DOWN: return Orientation.DOWN;
      case Direction.LEFT: return Orientation.LEFT;
      case Direction.RIGHT: return Orientation.RIGHT;
      default: return Orientation.UP;
    }
  });

  const pacmanClassesString = createMemo(() => {
    const s = props.pacmanStateAccessor();
    let cls = `pacman ${pacmanOrientation()}`;
    if (s.isChomping) cls += " pacman-chomp";
    return cls;
  });

  // for some things we'll have to mix in some inline styles I think
  return (
    <div class={pacmanClassesString()}  style={positionStyles()}/>
  );
};

export default Pacman;