package main

import (
    "encoding/json"
    "fmt"
    "html/template"
    "log"
    "net/http"
    "sync"
    "time"
)

// GINI - means "WHAT" in Igbo - the questioning parrot who loves to gossip
type GINI struct {
    Question string `json:"question"`
    Answer   string `json:"answer"`
    Language string `json:"language"`
    Gossip   string `json:"gossip"`
}

// GINIServer handles all the questions GINI asks
type GINIServer struct {
    questions []string
    gossip    []string
    mutex     sync.RWMutex
}

func NewGINIServer() *GINIServer {
    return &GINIServer{
        questions: []string{
            "Gini ka inweta?", // What do you have?
            "Kedu nke a?",     // What is this?
            "Gini?",           // What?
            "What are you building?",
            "What language is that?",
            "What does this code do?",
        },
        gossip: []string{
            "Did you hear about the new polyglot framework?",
            "I heard Python and Go are best friends now!",
            "Someone told me Rust never forgets...",
            "They say C is the grandfather of all languages!",
        },
    }
}

func (gs *GINIServer) ServeHTTP(w http.ResponseWriter, r *http.Request) {
    if r.URL.Path == "/api/gini" {
        gs.handleGINI(w, r)
        return
    }
    gs.handleIndex(w, r)
}

func (gs *GINIServer) handleGINI(w http.ResponseWriter, r *http.Request) {
    gs.mutex.RLock()
    defer gs.mutex.RUnlock()
    
    response := map[string]interface{}{
        "greeting": "Gini? (What?)",
        "questions": gs.questions,
        "gossip": gs.gossip,
        "languages": []string{"Igbo", "English", "Go", "Python", "C", "Rust"},
        "quote": "I ask 'What?' because I want to understand!",
    }
    
    w.Header().Set("Content-Type", "application/json")
    json.NewEncoder(w).Encode(response)
}

func (gs *GINIServer) handleIndex(w http.ResponseWriter, r *http.Request) {
    tmpl := `
<!DOCTYPE html>
<html>
<head>
    <title>GINI - The Questioning Parrot</title>
    <style>
        body {
            font-family: 'Courier New', monospace;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 20px;
            text-align: center;
        }
        .container {
            max-width: 800px;
            margin: 0 auto;
        }
        .gini-says {
            background: rgba(0,0,0,0.3);
            padding: 20px;
            border-radius: 10px;
            margin: 20px 0;
        }
        .parrot {
            font-size: 100px;
            margin: 20px;
        }
        .question {
            font-size: 24px;
            margin: 10px;
            padding: 10px;
            background: rgba(255,255,255,0.1);
            border-radius: 5px;
        }
        .definition {
            background: rgba(255,255,255,0.2);
            padding: 15px;
            border-radius: 10px;
            margin: 20px 0;
        }
        h1 { font-size: 48px; }
        h2 { font-size: 32px; }
    </style>
</head>
<body>
    <div class="container">
        <div class="parrot">🦜</div>
        <h1>GINI</h1>
        <h2>"Gini?" means "What?" in Igbo!</h2>
        
        <div class="definition">
            <h3>GINI = "WHAT" in Igbo Language</h3>
            <p>I'm a curious parrot who loves to ask questions!</p>
            <p>I gossip with polyglot programmers in Go, Python, C, Rust...</p>
            <p>Every conversation starts with "Gini?" (What?)</p>
        </div>
        
        <div class="gini-says">
            <h3>GINI asks:</h3>
            <div class="question">Gini ka inweta? (What do you have?)</div>
            <div class="question">What language are you coding in?</div>
            <div class="question">What's the latest gossip in tech?</div>
            <div class="question">What does your code do?</div>
        </div>
        
        <div class="gini-says">
            <h3>GINI gossips:</h3>
            <p>"I heard Python and Go are collaborating on something big!"</p>
            <p>"Did you know 'Gini' is how we say 'What' in Igbo?"</p>
            <p>"Someone told me Rust never has null pointer exceptions..."</p>
        </div>
        
        <p style="margin-top: 40px;">
            <strong>Fun Fact:</strong> In Igbo, asking "Gini?" is like asking "What?" in English.<br>
            I'm always asking questions because I love to learn!
        </p>
        
        <div style="margin-top: 40px;">
            <button onclick="askGINI()">Ask GINI Something!</button>
        </div>
    </div>
    
    <script>
        async function askGINI() {
            const response = await fetch('/api/gini');
            const data = await response.json();
            console.log('GINI responds:', data);
            alert('GINI says: ' + data.greeting + '\n\n' + data.quote);
        }
    </script>
</body>
</html>
    `
    
    t, _ := template.New("index").Parse(tmpl)
    t.Execute(w, nil)
}

func main() {
    server := NewGINIServer()
    
    fmt.Println("🦜 GINI Server Starting...")
    fmt.Println("==========================")
    fmt.Println("GINI = 'WHAT' in Igbo")
    fmt.Println("I love to ask questions and gossip!")
    fmt.Println("")
    fmt.Println("Server running on http://localhost:8080")
    fmt.Println("")
    fmt.Println("GINI asks: 'Gini ka inweta?' (What do you have?)")
    
    log.Fatal(http.ListenAndServe(":8080", server))
}
