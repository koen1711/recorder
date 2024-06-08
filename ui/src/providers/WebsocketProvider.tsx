import {createContext, MutableRefObject, useEffect, useRef, useState, Context} from "react";

// @ts-ignore
export const WebsocketContext : Context<[boolean, any, (s: string) => void]> = createContext([false, null, () => {}]);
//                                            ready, value, send

// Make sure to put WebsocketProvider higher up in
// the component tree than any consumers.
// @ts-ignore
export const WebsocketProvider = ({ children }) => {
    const [isReady, setIsReady] = useState(false);
    const [val, setVal] = useState(null);

    const ws : MutableRefObject<WebSocket|null> = useRef(null);

    useEffect(() => {
        const socket: WebSocket = new WebSocket("ws://" + window.location.host + "/ws");

        socket.onopen = () => setIsReady(true);
        socket.onclose = () => setIsReady(false);
        socket.onmessage = (event) => { setVal(event.data); };

        ws.current = socket;

        return () => {
            socket.close();
        };
    }, []);

    const ret = [isReady, val, (s: string) => ws.current?.send(s)];

    return (
        // @ts-ignore
        <WebsocketContext.Provider value={ret}>
            {children}
        </WebsocketContext.Provider>
    );
};