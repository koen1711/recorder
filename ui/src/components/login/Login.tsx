import {Button, Card, TextInput, Title} from "@tremor/react";
import React from "react";


export function LoginPage() {
    function sha512(str: string) {
        return crypto.subtle.digest("SHA-512", new TextEncoder("utf-8").encode(str)).then(buf => {
            return Array.prototype.map.call(new Uint8Array(buf), x=>(('00'+x.toString(16)).slice(-2))).join('');
        });
    }

    const [loggingIn, setLoggingIn] = React.useState(false);
    const [usernameError, setUsernameError] = React.useState(false);
    const [passwordError, setPasswordError] = React.useState(false);

    async function logIn() {
        // post request to /api/login, if successful put the token in local storage and redirect to the query of redirect_uri
        // if not successful, show an error message
        const username = (document.getElementById("username") as HTMLInputElement).value;
        const password = await sha512((document.getElementById("password") as HTMLInputElement).value);
        fetch("/api/login", {
            method: "POST",
            body: JSON.stringify({
                user: username,
                password: password
            }),
            headers: {
                "Content-Type": "application/json"
            }
        }).then((response) => {
            if (response.ok) {
                response.json().then((json) => {
                    localStorage.setItem("token", json.token);
                    // set a cookie with the token
                    document.cookie = `token=${json.token}; path=/`;
                    // look at the redirect_uri query parameter of the current url
                    // if it is not there, redirect to /
                    const queryString = window.location.search;
                    const urlParams = new URLSearchParams(queryString);
                    const redirect = urlParams.get("redirect_uri");
                    window.location.href = redirect ? redirect : "/";

                });
            } else {
                response.json().then((json) => {
                    if (json.error === "usernotfound") {
                        setUsernameError(true);
                        setPasswordError(false);
                    } else if (json.error === "passwordincorrect") {
                        setPasswordError(true);
                        setUsernameError(false);
                    }
                    setLoggingIn(false);
                });
            }
        });
    }

    return (
        <>
            <Card className={"w-1/3 left-1/3 top-1/3 absolute"}>
                <Title>Login</Title>
                <TextInput error={usernameError} errorMessage={"The username was incorrect"} id="username" disabled={loggingIn} placeholder="Username" type="text" className={"mt-2"} />
                <TextInput error={passwordError} errorMessage={"The password was incorrect"} id="password" disabled={loggingIn} placeholder="Password" type="password" className={"mt-2"} />
                <Button loading={loggingIn} className={"mt-2 w-1/6 mx-auto"} onClick={() => {setLoggingIn(true); logIn()}}>Login</Button>
            </Card>
        </>
    );
}