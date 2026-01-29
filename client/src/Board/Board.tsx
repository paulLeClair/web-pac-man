import {type Component, createEffect, createSignal} from 'solid-js';

// we'll probably just have a few different resolution pics of the game board and choose one
// based off the user's screen

const DEFAULT_TILE_SIZE_IN_PIXELS = 8;
const BOARD_WIDTH_IN_TILES = 28;
const BOARD_HEIGHT_IN_TILES = 36;

const BOARD_IMG = "/assets/Board/pacmaze-resized.jpg"

const Board: Component = () => {
  return (
    <section class="min-h-screen w-full bg-slate-950 text-slate-100 flex flex-col items-center justify-center px-6 py-10">
      <div class="mt-8 flex w-full items-center justify-center">
        <div class="relative aspect-[28/36] w-[min(90vw,calc(80vh*28/36))]">
          <div class="absolute inset-0 rounded-3xl bg-slate-900/70 ring-1 ring-slate-700 shadow-[0_24px_70px_-45px_rgba(0,0,0,0.9)]" />
          <img
            src={BOARD_IMG}
            alt="Pac-Man board"
            class="absolute inset-0 h-full w-full rounded-3xl object-contain"
          />
        </div>
      </div>
    </section>
  );
};

export default Board;
