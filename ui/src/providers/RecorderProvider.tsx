import React, {createContext, useState, Context} from "react";
import {WebsocketContext} from "./WebsocketProvider.tsx";

export type ChannelList = {
    channels: [
        {name: string, index: number, stereoLink: number, isLeft: boolean},
    ]
}

// @ts-ignore
export const RecorderContext : Context<[string, (s: string) => void, boolean|null, (s: boolean|null) => void, ChannelList, (s: ChannelList) => void]> = createContext(["", () => {}]);
//                                            ready, value, send


// @ts-ignore
export const RecorderProvider = ({ children }) => {
    const [val, setVal] = useState("");
    // @ts-ignore
    const [recording, setRecording]: [boolean|null, (s: boolean|null) => void] = useState(false);
    const [channelInfo, setChannelInfo] = useState({channels: []} as unknown as ChannelList);
    const ret = [val, (s: string) => setVal(s), recording, (s: boolean|null) => setRecording(s), channelInfo, setChannelInfo];

    const [ready, webSocketVal, send] = React.useContext(WebsocketContext);


    React.useEffect(() => {
        if (webSocketVal) {
            const json = JSON.parse(webSocketVal);
            if (json.type === 'audio-channels') {
                setChannelInfo(json);
            } else if (json.type === 'recorder-selected') {
                send(JSON.stringify({type: 'command', command: 'query-channels'}));
            }
        }
    }, [webSocketVal]);

    React.useEffect(() => {
        if (ready) {
            send(JSON.stringify({type: 'command', command: 'query-channels'}));
        }
    }, [ready]);

    return (
        // @ts-ignore
        <RecorderContext.Provider value={ret}>
            {children}
        </RecorderContext.Provider>
    );
};