import ReactDOM from 'react-dom/client'
import App from './App.tsx'
import './index.css'
import { WebsocketProvider } from './providers/WebsocketProvider.tsx'
import { RecorderProvider } from './providers/RecorderProvider.tsx'

ReactDOM.createRoot(document.getElementById('root')!).render(
    <WebsocketProvider>
        <RecorderProvider>
            <App />
        </RecorderProvider>
    </WebsocketProvider>
)
