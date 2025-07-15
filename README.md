# Menu Manager
Easy and flexible menu manager for ESP-IDF


```mermaid
---
config:
    title: Menu Manager
---
flowchart TD
    start((menu init))
    createQueue[Create Queue qCommand]
    waitQueue[Wait Queue qCommand]
    showDisplay[Show menu]
    
    funtionInExecution{Is there a funtion in execution?}
    isCommandBack{Is command back?}
    exitFuntion[Exit funtion in execution]
    
    swithCasesInput{Command}
    navigateUP[Increments the index]
    navigateDOWN[Decrements the index]
    navigateBACK[return to parent menu]
    
    navigateSELECTION{Is there a funtion?}
    selectSubMenu[Select sub menu]
    executeFuntion[Execute funtion]
    
    start --> createQueue
    createQueue --> waitQueue
    waitQueue --> funtionInExecution
    funtionInExecution --"Yes"--> isCommandBack
    isCommandBack --"Yes"--> exitFuntion
    funtionInExecution --"No"--> swithCasesInput
    
    swithCasesInput --"UP"--> navigateUP
    swithCasesInput --"DOWN"--> navigateDOWN
    swithCasesInput --"BACK"--> navigateBACK
    swithCasesInput --"SELECT"--> navigateSELECTION
    
    exitFuntion --> showDisplay
    navigateUP --> showDisplay
    navigateDOWN --> showDisplay
    navigateBACK --> showDisplay
    
    navigateSELECTION --"No"--> selectSubMenu --> showDisplay
    navigateSELECTION --"Yes"--> executeFuntion --> waitQueue
    
    showDisplay --> waitQueue

    %% Estilos e cores
    classDef startEnd fill:#e1f5fe,stroke:#01579b,stroke-width:3px,color:#000
    classDef queue fill:#f3e5f5,stroke:#4a148c,stroke-width:2px,color:#000
    classDef display fill:#e8f5e8,stroke:#2e7d32,stroke-width:2px,color:#000
    classDef decision fill:#fff3e0,stroke:#e65100,stroke-width:2px,color:#000
    classDef navigation fill:#fce4ec,stroke:#ad1457,stroke-width:2px,color:#000
    classDef execution fill:#ffebee,stroke:#c62828,stroke-width:2px,color:#000
    classDef selection fill:#e0f2f1,stroke:#00695c,stroke-width:2px,color:#000

    class start startEnd
    class createQueue,waitQueue queue
    class showDisplay display
    class funtionInExecution,isCommandBack,swithCasesInput,navigateSELECTION decision
    class navigateUP,navigateDOWN,navigateBACK navigation
    class exitFuntion,executeFuntion execution
    class selectSubMenu selection```

