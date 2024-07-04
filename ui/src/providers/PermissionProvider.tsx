import React, {createContext, MutableRefObject, useEffect, useRef, useState, Context} from "react";
import {WebsocketContext} from "./WebsocketProvider.tsx";

// @ts-ignore
export const PermissionContext : Context<[boolean, boolean, boolean, boolean]> = createContext([false, null, () => {}]);
//                                            ready, value, send

// Make sure to put WebsocketProvider higher up in
// the component tree than any consumers.
// @ts-ignore
export const PermissionProvider = ({ children }) => {
    const [ready, webSocketVal, send] = React.useContext(WebsocketContext);

    const [ loggedIn, setLoggedIn ] = React.useState(false);

    const [allowUnregisteredRecording, setAllowUnregisteredRecording]: [boolean, (s: boolean) => void] = React.useState(false);
    const [allowUnregisteredStreaming, setAllowUnregisteredStreaming]: [boolean, (s: boolean) => void] = React.useState(false);
    const [allowUnregisteredConfig, setAllowUnregisteredConfig]: [boolean, (s: boolean) => void] = React.useState(false);

    React.useEffect(() => {
        if (webSocketVal) {
            const json = JSON.parse(webSocketVal);
            if (json.type === 'permissions' && !loggedIn) {
                setAllowUnregisteredRecording(json.allowUnregisteredRecording);
                setAllowUnregisteredStreaming(json.allowUnregisteredStreaming);
                setAllowUnregisteredConfig(json.allowUnregisteredConfig);
            } else if (json.type === 'login') {
                setLoggedIn(true);
            }
        }
    }, [webSocketVal]);

    React.useEffect(() => {
        if (loggedIn) {
            setAllowUnregisteredRecording(true);
            setAllowUnregisteredStreaming(true);
            setAllowUnregisteredConfig(true);
        }
    }, [loggedIn]);

    React.useEffect(() => {
        if (ready && localStorage.getItem('token')) {
            // wait for 500ms
            setTimeout(() => send(JSON.stringify({type: 'login', token: localStorage.getItem('token')})), 500);
        }
    }, [ready]);

    return (
        // @ts-ignore
        <PermissionContext.Provider value={[allowUnregisteredRecording, allowUnregisteredStreaming, allowUnregisteredConfig]}>
            {children}
        </PermissionContext.Provider>
    );
};