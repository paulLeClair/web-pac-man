import {Component, createSignal} from 'solid-js';
import './Pacman.css'
import {Direction, EntityState} from "../App";

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
  pacmanStateAccessor: () => PacmanState
}

const Pacman: Component<PacmanProps> = (props) => {
  const positionStyles = {
    top: props.pacmanStateAccessor().y.toString() + "px",
    left: props.pacmanStateAccessor().x.toString() + "px"
  }

  let pacmanOrientation = Orientation.UP;
  switch (props.pacmanStateAccessor().orientation) {
    case Direction.UP: pacmanOrientation = Orientation.UP; break;
    case Direction.DOWN: pacmanOrientation = Orientation.DOWN; break;
    case Direction.LEFT: pacmanOrientation = Orientation.LEFT; break;
    case Direction.RIGHT: pacmanOrientation = Orientation.RIGHT; break;
  }

  let pacmanClassesString = "pacman " + pacmanOrientation;
  if (props.pacmanStateAccessor().isChomping) {
    pacmanClassesString += " pacman-chomp"
  }

  // for some things we'll have to mix in some inline styles I think
  return (
    <div class={pacmanClassesString}  style={positionStyles}/>
  );
};

export default Pacman;