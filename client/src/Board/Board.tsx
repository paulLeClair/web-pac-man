import {type Component, Show, createMemo, JSX} from 'solid-js';
import './Board.css'
import {SideOfBoardPadding, TopOfBoardPadding} from "../App";

const BOARD_IMG = "/assets/Board/pacmaze-resized.jpg"

// might make sense to move items off to their own file because they likely will need a
// decent amount of functionality (eg creating the little score popup when you grab a special item)
interface ItemProps {
    type: number,
    x: number,
    y: number
}

enum ItemType {
    UNKNOWN = 0,
    DOT = 10,
    ENERGIZER = 50,
    CHERRY = 100,
    STRAWBERRY = 300,
    ORANGE = 500,
    APPLE = 700,
    MELON = 1000,
    GALAXIAN = 2000,
    BELL = 3000,
    KEY = 5000
}

const Item: Component<ItemProps> = (props) => {
    let itemTypeClassStr = ""
    let cellXOffset = 0;
    let cellYOffset = 0;
    switch (props.type) {
        case (ItemType.DOT): {
            itemTypeClassStr = "dot"
            cellXOffset = 4;
            cellYOffset = 4;
            break
        }
        case (ItemType.ENERGIZER): {
            itemTypeClassStr = "energizer"
            cellXOffset = 2;
            cellYOffset = 2;
            break
        }
        case (ItemType.CHERRY): {
            itemTypeClassStr = "special-item cherry"
            break
        }
        case (ItemType.STRAWBERRY): {
            itemTypeClassStr = "special-item strawberry"
            break
        }
        case (ItemType.ORANGE): {
            itemTypeClassStr = "special-item orange"
            break
        }
        case (ItemType.APPLE): {
            itemTypeClassStr = "special-item apple"
            break
        }
        case (ItemType.MELON): {
            itemTypeClassStr = "special-item melon"
            break
        }
        case (ItemType.GALAXIAN): {
            itemTypeClassStr = "special-item galaxian"

            break
        }
        case  (ItemType.BELL): {
            itemTypeClassStr = "special-item bell"

            break
        }
        case (ItemType.KEY): {
            itemTypeClassStr = "special-item key"
            break
        }
        default:
            return (<div/>)
    }

    const positionStyles = {
        top: `${props.y + TopOfBoardPadding + cellYOffset}px`,
        left: `${props.x + SideOfBoardPadding + cellXOffset}px`,
    }

    return (
        <div class={itemTypeClassStr} style={positionStyles} />
    )
}

interface BoardProps {
    children?: JSX.Element;
    itemsAccessor: () => Map<number, number>;
    hidden: boolean,
}

const Board: Component<BoardProps> = (props) => {
    const itemMemo = createMemo(() => {
        const i = props.itemsAccessor();

        let result: JSX.Element[] = [];
        for (const [packedCoordinates, itemType] of i) {
            let unpackedX = packedCoordinates >> 16;
            let unpackedY = (packedCoordinates << 16) >> 16;
            result.push(<Item type={itemType} x={unpackedX} y={unpackedY}/>)
        }
        return result;
    })

    return (
        <Show when={!props.hidden}>
            <section
                class="w-fit bg-black text-slate-100">
                <div class="flex items-center justify-center">
                    <div class="relative aspect-28/36 w-[min(90vw,calc(80vh*28/36))] max-w-[224px]">
                        <div
                            class="absolute inset-0 3xl bg-black "/>
                        <img
                            src={BOARD_IMG}
                            alt="Pac-Man board"
                            class="absolute inset-0 h-full w-full 3xl object-contain"
                        />
                        {itemMemo()}
                        {props.children}
                    </div>
                </div>
            </section>
        </Show>
    );
};

export default Board;
