const quizData = [
    {
    question: "What is hydroponics?",
    choices: [
        "Growing plants without soil, using nutrient-rich water",
        "A method of planting seeds in soil pots",
        "A technique for harvesting rainwater",
        "Using pesticides to improve plant growth"
    ],
    correctIndex: 0
    },
    {
    question: "Why is lettuce popular for hydroponic farming?",
    choices: [
        "It requires a long growing time",
        "It grows quickly and thrives in water-based systems",
        "It needs a lot of soil nutrients",
        "It can't be grown hydroponically"
    ],
    correctIndex: 1
    }
];

let currentQuestionIndex = 0;
let answered = false;

const questionText = document.getElementById('question-text');
const choicesList = document.getElementById('choices-list');
const feedback = document.getElementById('feedback');
const nextBtn = document.getElementById('next-btn');
const restartBtn = document.getElementById('restart-btn');

function loadQuestion() {
    answered = false;
    nextBtn.disabled = true;
    feedback.textContent = '';
    const current = quizData[currentQuestionIndex];
    questionText.textContent = current.question;
    choicesList.innerHTML = '';
    current.choices.forEach((choice, index) => {
    const li = document.createElement('li');
    const btn = document.createElement('button');
    btn.classList.add('choice-btn');
    btn.textContent = choice;
    btn.setAttribute('data-index', index);
    btn.type = 'button';
    btn.addEventListener('click', () => selectAnswer(index, btn));
    li.appendChild(btn);
    choicesList.appendChild(li);
    });
    nextBtn.style.display = 'inline-block';
    restartBtn.style.display = 'none';
}

function selectAnswer(index, btn) {
    if (answered) return;
    answered = true;
    const current = quizData[currentQuestionIndex];
    const buttons = document.querySelectorAll('.choice-btn');
    buttons.forEach(b => b.disabled = true);
    if (index === current.correctIndex) {
    btn.classList.add('correct');
    feedback.textContent = 'Correct! 🎉';
    feedback.style.color = '#4caf50';
    } else {
    btn.classList.add('incorrect');
    feedback.textContent = `Incorrect! The correct answer is: "${current.choices[current.correctIndex]}"`;
    feedback.style.color = '#e74c3c';
    // Highlight correct answer
    buttons[current.correctIndex].classList.add('correct');
    }
    nextBtn.disabled = false;
}

nextBtn.addEventListener('click', () => {
    currentQuestionIndex++;
    if (currentQuestionIndex < quizData.length) {
    loadQuestion();
    } else {
    showEndScreen();
    }
});

restartBtn.addEventListener('click', () => {
    currentQuestionIndex = 0;
    loadQuestion();
});

function showEndScreen() {
    questionText.textContent = "Great job! You have completed the quiz.";
    choicesList.innerHTML = '';
    feedback.textContent = '';
    nextBtn.style.display = 'none';
    restartBtn.style.display = 'inline-block';
}

// Initialize quiz
loadQuestion();