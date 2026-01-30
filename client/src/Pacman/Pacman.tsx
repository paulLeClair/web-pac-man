import {Component, createSignal} from 'solid-js';
import './Pacman.css'

enum Orientation {
  UP = "orientation-up",
  DOWN = "orientation-down",
  LEFT = "orientation-left",
  RIGHT = "orientation-right"
}
const Pacman: Component = () => {
  const [xPixel, setPixelPositionX] = createSignal(0)
  const [yPixel, setPixelPositionY] = createSignal(0)

  const [orientation, setOrientation] = createSignal(Orientation.UP)

  const [isChomping, setIsChomping] = createSignal(true)

  const positionStyles = {
    top: xPixel().toString() + "px",
    left: yPixel().toString() + "px"
  }

  let pacmanClassesString = "pacman " + orientation();
  if (isChomping()) {
    pacmanClassesString += " pacman-chomp"
  }

  // for some things we'll have to mix in some inline styles I think
  return (
    <div class={pacmanClassesString}  style={positionStyles}></div>
  );
};

export default Pacman;